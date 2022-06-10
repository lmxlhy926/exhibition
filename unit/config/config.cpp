
#include <thread>
#include <atomic>
#include <cstdio>

#include "siteService/nlohmann/json.hpp"
#include "siteService/service_site_manager.h"

#include "socket/httplib.h"
#include "serviceRequestHandler.h"
#include "qlibc/FileUtils.h"
#include "mqttPayloadHandle.h"
#include "secretUtils.h"
#include "paramconfig.h"
#include "messageSubscribeHandler.h"

using namespace std;
using namespace servicesite;
using namespace httplib;
using namespace std::placeholders;
using json = nlohmann::json;

static const string CONFIG_SITE_ID = "config";
static const string CONFIG_SITE_ID_NAME = "整体配置站点";


int main(int argc, char* argv[]) {

    httplib::ThreadPool threadPool_(30);
    std::atomic<bool> http_server_thread_end(false);

    //设置配置文件加载路径
    configParamUtil* configPathPtr = configParamUtil::getInstance();
    configPathPtr->setConfigPath(string(argv[1]));

    //加载http服务器配置信息，初始化云端对接类
    QData httpConfigData = configPathPtr->getCloudServerData();
    const string dataDirPath = configPathPtr->getconfigPath();
    cloudUtil::getInstance()->init(httpConfigData.getString("ip"), httpConfigData.getInt("port"), dataDirPath);

    //开启线程前，所有的单例已经创建完毕
    //开启线程，阻塞进行电视加入大白名单
    threadPool_.enqueue([](){
        cloudUtil::getInstance()->joinTvWhite();
    });


    //加载mqtt配置信息
    QData mqttConfigData = configParamUtil::getInstance()->getMqttConfigData();
    std::string mqttServer = mqttConfigData.getString("server");
    int mqttPort = mqttConfigData.getInt("port");
    std::string mqttUsername = mqttConfigData.getString("username");
    std::string mqttPassword = mqttConfigData.getString("password");

    //加载基础参数信息
    QData baseInfoData = configParamUtil::getInstance()->getBaseInfo();
    string domainID = baseInfoData.getString("domainID");

    string clientID = domainID.empty() ?
            "config" + std::to_string(time(nullptr)) : "config" + domainID + std::to_string(time(nullptr));

    //配置mqtt客户端、设置处理回调、设置预处理回调、订阅主题
    mqttClient mc;
    mc.paramConfig(mqttServer, mqttPort, mqttUsername, mqttPassword, clientID);
    if(!domainID.empty()){
        mc.subscribe("edge/" + domainID + "/device/domainWhite");
    }
    mc.setDefaultHandler(mqttPayloadHandle::handle);
    mc.addDataHooker([](const std::string& topic, void *payload, int payloadLen, char* buffer, int* len)->bool{
        const string in = string(reinterpret_cast<char *>(payload), 0, payloadLen);
        string out;
        const uint8_t key[] = "123456asdfgh1234";
        lhytemp::secretUtil::ecb_decrypt_withPadding(in, out, key);

        strcpy(buffer, out.data());
        *len = static_cast<int>(out.size());

        return true;
    });
    mc.connect();

    // 创建 serviceSiteManager 对象, 单例
    ServiceSiteManager* serviceSiteManager = ServiceSiteManager::getInstance();
    serviceSiteManager->setServerPort(ConfigSitePort);

    //注册请求场景列表处理函数
    serviceSiteManager->registerServiceRequestHandler(SCENELIST_REQUEST_SERVICE_ID,sceneListRequest_service_request_handler);
    //注册子设备注册处理函数
    serviceSiteManager->registerServiceRequestHandler(SUBDEVICE_REGISTER_SERVICE_ID, subDeviceRegister_service_request_handler);
    //注册获取家庭域Id处理函数
    serviceSiteManager->registerServiceRequestHandler(DOMAINID_REQUEST_SERVICE_ID, domainIdRequest_service_request_handler);
    //注册安装师傅信息上传请求函数
    serviceSiteManager->registerServiceRequestHandler(ENGINEER_REQUEST_SERVICE_ID,
                                                      [&](const Request& request, Response& response) -> int{
        return engineer_service_request_handler(mc, request, response);
    });
    //获取白名单列表
    serviceSiteManager->registerServiceRequestHandler(WHITELIST_REQUEST_SERVICE_ID,
                                                      [&](const Request& request, Response& response) -> int{
        return whiteList_service_request_handler(request, response);
    });
    //获取所有的设备列表
    serviceSiteManager->registerServiceRequestHandler(GETALLLIST_REQUEST_SERVICE_ID,
                                                      [&](const Request& request, Response& response) -> int{
        return getAllDeviceList_service_request_handler(request, response);
    });
    //让电视发声
    serviceSiteManager->registerServiceRequestHandler(TVSOUND_REQUEST_SERVICE_ID,
                                                      [&](const Request& request, Response& response) -> int{
        return tvSound_service_request_handler(request, response);
    });



#if 0
    //注册订阅消息messageID;
    serviceSiteManager->registerMessageId(TVADAPTER_DEVICE_STATUS_MESSAGE_ID);
    serviceSiteManager->registerMessageId(RADAR_DEVICE_STATUS_MESSAGE_ID);

    //注册messageID对应的handler;
    serviceSiteManager->registerMessageHandler(TVADAPTER_DEVICE_STATUS_MESSAGE_ID, deviceStatus);
    serviceSiteManager->registerMessageHandler(RADAR_DEVICE_STATUS_MESSAGE_ID, deviceStatus);

    threadPool_.enqueue([&](){
        while(true){
            int code1, code2;
            std::vector<string> messageIdList1, messageIdList2;
            messageIdList1.push_back(TVADAPTER_DEVICE_STATUS_MESSAGE_ID);
            messageIdList2.push_back(RADAR_DEVICE_STATUS_MESSAGE_ID);

            code1 = serviceSiteManager->subscribeMessage(RequestIp, AdapterPort, messageIdList1);
            code2 = serviceSiteManager->subscribeMessage(RequestIp, 60003, messageIdList2);

            if (code1 == ServiceSiteManager::RET_CODE_OK && code2 == ServiceSiteManager::RET_CODE_OK) {
                printf("subscribeMessage ok.\n");
                break;
            }

            std::this_thread::sleep_for(std::chrono::seconds(3));
            printf("subscribed failed....., start to subscribe in 3 seconds\n");
        }
    });
#endif

    // 站点监听线程启动
    threadPool_.enqueue([&](){
        // 启动服务器，参数为端口， 可用于单独的开发调试
        int code = serviceSiteManager->start();

        // 通过注册的方式启动服务器， 需要提供site_id, site_name, port
    	//code = serviceSiteManager->startByRegister(TEST_SITE_ID_1, TEST_SITE_NAME_1, 9001);

        if (code != 0) {
            printf("start error. code = %d\n", code);
        }

        http_server_thread_end.store(true);
    });

    std::this_thread::sleep_for(std::chrono::seconds(5));

    while(true){
        if (http_server_thread_end.load()){
            printf("http end abnormally....\n");
            break;
        }
        std::this_thread::sleep_for(std::chrono::seconds(30));
    }

    return -1;
}
