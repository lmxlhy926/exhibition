
#include <thread>
#include <atomic>
#include <cstdio>

#include "siteService/nlohmann/json.hpp"
#include "siteService/service_site_manager.h"
#include "serviceRequestHandler.h"
#include "messageSubscribeHandler.h"
#include "qlibc/FileUtils.h"
#include "util/mqttPayloadHandle.h"
#include "util/secretUtils.h"
#include "param.h"
#include "log/Logging.h"

using namespace std;
using namespace servicesite;
using namespace httplib;
using namespace std::placeholders;
using json = nlohmann::json;


int main(int argc, char* argv[]) {

    if(argc != 2){
        LOG_RED << "Usage Error.....";
        LOG_PURPLE << "Try again with the format: config <DirPath>";
        return 0;
    }

    //. 设置线程池
    httplib::ThreadPool threadPool_(10);
    std::atomic<bool> http_server_thread_end(false);

    //. 配置本站点启动信息
    ServiceSiteManager* serviceSiteManager = ServiceSiteManager::getInstance();
    serviceSiteManager->setServerPort(ConfigSitePort);
    serviceSiteManager->setSiteIdSummary(CONFIG_SITE_ID, CONFIG_SITE_ID_NAME);

    //set site supported subscribed message
    serviceSiteManager->registerMessageId(WHITELIST_MESSAGE_ID);
    serviceSiteManager->registerMessageId(RECEIVED_WHITELIST_ID);


   //. 设置配置文件加载路径
    configParamUtil* configPathPtr = configParamUtil::getInstance();
    configPathPtr->setConfigPath(string(argv[1]));


    //. 初始化cloudUtil, 加载http服务器配置信息
    QData httpConfigData = configPathPtr->getCloudServerData();
    const string dataDirPath = configPathPtr->getconfigPath();
    cloudUtil::getInstance()->init(httpConfigData.getString("ip"), httpConfigData.getInt("port"), dataDirPath);

    //. 开启线程，阻塞进行电视加入大白名单
    threadPool_.enqueue([](){
        cloudUtil::getInstance()->joinTvWhite();
    });


    //. 获取mqtt配置参数
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
    serviceSiteManager->registerServiceRequestHandler(WHITELIST_REQUEST_SERVICE_ID,whiteList_service_request_handler);
    //更新白名单列表
    serviceSiteManager->registerServiceRequestHandler(WHITELIST_UPDATE_REQUEST_SERVICE_ID,whiteList_update_service_request_handler);
    //保存白名单列表
    serviceSiteManager->registerServiceRequestHandler(WHITELIST_SAVE_REQUEST_SERVICE_ID,whiteList_save_service_request_handler);
    //获取灯控设备列表
    serviceSiteManager->registerServiceRequestHandler(GETALLLIST_REQUEST_SERVICE_ID,getAllDeviceList_service_request_handler);
    //让电视发声
    serviceSiteManager->registerServiceRequestHandler(TVSOUND_REQUEST_SERVICE_ID,tvSound_service_request_handler);
    //注册messageID对应的handler;
    serviceSiteManager->registerMessageHandler(REGISTERAGAIN_MESSAGE_ID, register2QuerySite);

#if 0
    threadPool_.enqueue([&](){
        while(true){
            int code;
            std::vector<string> messageIdList;
            messageIdList.push_back(REGISTERAGAIN_MESSAGE_ID);
            code = serviceSiteManager->subscribeMessage(RequestIp, QuerySitePort, messageIdList);

            if (code == ServiceSiteManager::RET_CODE_OK) {
                printf("subscribeMessage REGISTERAGAIN_MESSAGE_ID ok.\n");
                break;
            }

            std::this_thread::sleep_for(std::chrono::seconds(3));
            printf("subscribed REGISTERAGAIN_MESSAGE_ID failed....., start to subscribe in 3 seconds\n");
        }
    });
#endif

    // 站点监听线程启动
    threadPool_.enqueue([&](){
        while(true){
            //自启动方式
            int code = serviceSiteManager->start();
            //注册启动方式
//            int code = serviceSiteManager->startByRegister();
            if(code != 0){
                LOG_RED << "===>configSite startByRegister error, code = " << code;
                LOG_RED << "===>configSite startByRegister in 3 seconds....";
                std::this_thread::sleep_for(std::chrono::seconds(3));
            }else{
                LOG_RED << "===>configSite startByRegister successfully.....";
                break;
            }
        }
    });

    while(true){
        std::this_thread::sleep_for(std::chrono::seconds(60 *10));
    }

    return 0;
}
