
#include <thread>
#include <unistd.h>
#include <atomic>
#include <cstdio>
#include <functional>

#include "siteService/nlohmann/json.hpp"
#include "siteService/service_site_manager.h"

#include "socket/httplib.h"
#include "serviceRequestHandler.h"
#include "subscribeMessageHandler.h"
#include "socket/socketServer.h"
#include "cloudUtil.h"
#include "configParamUtil.h"
#include "qlibc/FileUtils.h"
#include "mqtt/mqttClient.h"

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
    cloudUtil::getInstance()->init(httpServerIp, serverPort, dataDirPath);

    //开启线程，电视加入大白名单
    threadPool_.enqueue([](){
        cloudUtil::getInstance()->joinTvWhite();
    });

    //开启socketServer
    string ip = "127.0.0.1";
    int port = 60003;
    socketServer server;
    if(server.start(ip, port)){
        std::cout << "---bind successfully---" << std::endl;
        server.listen();
    }else{
        std::cout << "---bind failed---" << std::endl;
    }

    //启动mqtt,订阅主题，发布
    QData mqttConfigData = configParamUtil::getInstance()->getMqttConfigData();
    std::string mqttServer = mqttConfigData.getString("server");
    int mqttPort = mqttConfigData.getInt("port");
    std::string mqttUsername = mqttConfigData.getString("username");
    std::string mqttPassword = mqttConfigData.getString("password");

    mqttClient mc;
    mc.paramConfig(mqttServer, mqttPort, mqttUsername, mqttPassword, "subscribe");
    if(mc.connect()){
        std::cout << "----connect successfully---" << std::endl;
    }
    mc.subscribe("abc");

    // 创建 serviceSiteManager 对象, 单例
    ServiceSiteManager* serviceSiteManager = ServiceSiteManager::getInstance();

    // 注册 Service 请求处理 handler， 有两个 Service
    serviceSiteManager->registerServiceRequestHandler(SCENELIST_REQUEST_SERVICE_ID,sceneListRequest_service_request_handler);
    serviceSiteManager->registerServiceRequestHandler(SUBDEVICE_REGISTER_SERVICE_ID, subDeviceRegister_service_request_handler);
    serviceSiteManager->registerServiceRequestHandler(DOMAINID_REQUEST_SERVICE_ID, domainIdRequest_service_request_handler);
    serviceSiteManager->registerServiceRequestHandler(ENGINEER_REQUEST_SERVICE_ID, engineer_service_request_handler);
    serviceSiteManager->registerServiceRequestHandler(GETDEVICELIST_REQUEST_SERVICE_ID, getDeviceList_service_request_handler);
    serviceSiteManager->registerServiceRequestHandler(GETTVINFO_REQUEST_SERVICE_ID, getTvInfo_service_request_handler);
    serviceSiteManager->registerServiceRequestHandler(CONTROLDEVICE_REQUEST_SERVICE_ID, controlDevice_service_request_handler);
    serviceSiteManager->registerServiceRequestHandler(REDARREPORT_REQUEST_SERVICE_ID, radarReportEnable_service_request_handler);

    // 注册支持的消息ID
    serviceSiteManager->registerMessageId(DEVICESTATUS_MESSAGE_ID);
    serviceSiteManager->registerMessageId(RADAR_MESSAGE_ID);
    serviceSiteManager->registerMessageId(MICPANELMESSAGE_ID);

    // 注册消息ID对应的handler
    serviceSiteManager->registerMessageHandler(DEVICESTATUS_MESSAGE_ID,
                                               [&](const Request& request){
        deviceStatus_message_handler(server, request);
    });
    serviceSiteManager->registerMessageHandler(DEVICESTATUS_MESSAGE_ID,
                                               [&](const Request& request){
        radar_message_handler(server, request);
    });
    serviceSiteManager->registerMessageHandler(DEVICESTATUS_MESSAGE_ID,
                                               [&](const Request& request){
        micpanel_message_handler(server, request);
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

    // 订阅消息, 需要传入订阅站点的IP、端口号、消息ID列表
    std::vector<string> messageIdList;
    messageIdList.push_back(DEVICESTATUS_MESSAGE_ID);
    messageIdList.push_back(RADAR_MESSAGE_ID);
    messageIdList.push_back(MICPANELMESSAGE_ID);
    int  code = serviceSiteManager->subscribeMessage("127.0.0.1", 60001, messageIdList);
    if (code == ServiceSiteManager::RET_CODE_OK) {
        printf("subscribeMessage ok.\n");
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
