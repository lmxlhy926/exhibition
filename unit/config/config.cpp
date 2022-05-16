
#include <thread>
#include <unistd.h>
#include <atomic>
#include <cstdio>
#include <functional>

#include "siteService/nlohmann/json.hpp"
#include "siteService/service_site_manager.h"

#include "socket/httplib.h"
#include "serviceRequestHandler.h"
#include "socket/socketServer.h"
#include "cloudUtil.h"
#include "common/configParamUtil.h"
#include "qlibc/FileUtils.h"
#include "mqtt/mqttClient.h"
#include "mqttPayloadHandle.h"
#include "myutil.h"

using namespace std;
using namespace servicesite;
using namespace httplib;
using namespace std::placeholders;
using json = nlohmann::json;

static const string CONFIG_SITE_ID = "config";
static const string CONFIG_SITE_ID_NAME = "整体配置站点";


void publish_message(void){
//    json message_json = {
//            {"message_id", "test_message_id_1"},
//            {"content", {
//                                   "some_data", 123
//                           }}
//    };
//
//    ServiceSiteManager* serviceSiteManager = ServiceSiteManager::getInstance();
//
//    // 把要发布的消息 json 字符串传入即可， 由库来向订阅过的站点发送消息
//    serviceSiteManager->publishMessage(TEST_MESSAGE_ID_1, message_json.dump());
//
//    message_json["message_id"] = "test_message_id_2";
//    serviceSiteManager->publishMessage(TEST_MESSAGE_ID_2, message_json.dump());
}



int main(int argc, char* argv[]) {

    httplib::ThreadPool threadPool_(100);
    std::atomic<bool> http_server_thread_end(false);

    //设置配置文件加载路径
    configParamUtil* configPathPtr = configParamUtil::getInstance();
    configPathPtr->setConfigPath(string(argv[1]));

    //加载http服务器配置信息，初始化云端对接类
    const string dataDirPath = configPathPtr->getconfigPath();
    qlibc::QData httpconfigData;
    httpconfigData.loadFromFile(FileUtils::contactFileName(dataDirPath, "httpconfig.json"));
    string httpServerIp = httpconfigData.getString("ip");
    int serverPort = httpconfigData.getInt("port");
    //初始化云端对接类
    cloudUtil::getInstance()->init(httpServerIp, serverPort, dataDirPath);

    //开启线程，电视加入大白名单
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
        lhytemp::myutil::ecb_decrypt_withPadding(in, out, key);

        strcpy(buffer, out.data());
        *len = static_cast<int>(out.size());

        return true;
    });
    mc.connect();


    // 创建 serviceSiteManager 对象, 单例
    ServiceSiteManager* serviceSiteManager = ServiceSiteManager::getInstance();

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

    // 站点监听线程启动
    threadPool_.enqueue([&](){
        // 启动服务器，参数为端口， 可用于单独的开发调试
        int code = serviceSiteManager->start(60002);

        // 通过注册的方式启动服务器， 需要提供site_id, site_name, port
//    	code = serviceSiteManager->startByRegister(TEST_SITE_ID_1, TEST_SITE_NAME_1, 9001);

        if (code != 0) {
            printf("start error. code = %d\n", code);
        }

        http_server_thread_end.store(true);
    });

    sleep(2);

    if (http_server_thread_end.load()) {
        printf("启动 http 服务器线程错误.\n");
        return -1;
    }


    while(true){
        if (http_server_thread_end){
            printf("http end abnormally....\n");
            break;
        }
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }

    return -1;
}
