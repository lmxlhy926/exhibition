
#include <thread>
#include <unistd.h>
#include <atomic>
#include <cstdio>
#include "socket/httplib.h"
#include "siteService/nlohmann/json.hpp"
#include "siteService/service_site_manager.h"

using namespace std;
using namespace servicesite;

using json = nlohmann::json;

const string CONFIG_SITE_ID = "config";
const string CONFIG_SITE_ID_NAME = "整体配置站点";

//服务ID, 对内提供
const string SCENELIST_REQUEST_SERVICE_ID = "";
const string SUBDEVICE_REGISTER_SERVICE_ID = "";
const string DOMAINID_REQUEST_SERVICE_ID = "";
//服务ID，对外提供
const string ENGINEER_REQUEST_SERVICE_ID = "";
const string GETDEVICELIST_REQUEST_SERVICE_ID = "";
const string GETTVINFO_REQUEST_SERVICE_ID = "";
const string CONTROLDEVICE_REQUEST_SERVICE_ID = "";
const string REDARREPORT_REQUEST_SERVICE_ID = "";

//消息ID
const string DEVICESTATUS_MESSAGE_ID = "";
const string RADAR_MESSAGE_ID = "";
const string MICPANELMESSAGE_ID = "";

// test_service_id_1 服务请求处理函数
int sceneListRequest_service_request_handler(const Request& request, Response& response) {
    // HTTP库已判断字符串能否转成 JSON

    // 请求的json字符串位于request.body
    auto request_json = json::parse(request.body);

    printf("request:\n%s\n", request_json.dump(4).c_str());

    // 服务反馈
    json response_json = {
            {"code", 0},
            {"error", "ok"},
            {"response", {
                             {"test_data", "from test_service_id_1"}
                     }}
    };
    response.set_content(response_json.dump(), "text/plain");

    return 0;
}

int subDeviceRegister_service_request_handler(const Request& request, Response& response) {

    return 0;
}

int domainIdRequest_service_request_handler(const Request& request, Response& response) {

    return 0;
}

int engineer_service_request_handler(const Request& request, Response& response) {

    return 0;
}

int getDeviceList_service_request_handler(const Request& request, Response& response) {

    return 0;
}

int getTvInfo_service_request_handler(const Request& request, Response& response) {

    return 0;
}

int controlDevice_service_request_handler(const Request& request, Response& response) {

    return 0;
}

int radarReportEnable_service_request_handler(const Request& request, Response& response) {

    return 0;
}


// test_message_id_1 消息处理函数
void deviceStatus_message_handler(const Request& request) {

}

void radar_message_handler(const Request& request) {

}

void micpanel_message_handler(const Request& request) {

}


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

// 标记线程错误
std::atomic<bool> http_server_thread_end(false);

void http_server_thread_func() {
    int code;

    ServiceSiteManager* serviceSiteManager = ServiceSiteManager::getInstance();

    // 启动服务器，参数为端口， 可用于单独的开发调试
    code = serviceSiteManager->start(60002);

    // 通过注册的方式启动服务器， 需要提供site_id, site_name, port
//	code = serviceSiteManager->startByRegister(TEST_SITE_ID_1, TEST_SITE_NAME_1, 9001);

    if (code != 0) {
        printf("start error. code = %d\n", code);
    }

    http_server_thread_end = true;
}

int main(int argc, char* argv[]) {
    // 创建 serviceSiteManager 对象, 单例
    ServiceSiteManager* serviceSiteManager = ServiceSiteManager::getInstance();

    // 注册 Service 请求处理 handler， 有两个 Service
    serviceSiteManager->registerServiceRequestHandler(SCENELIST_REQUEST_SERVICE_ID, sceneListRequest_service_request_handler);
    serviceSiteManager->registerServiceRequestHandler(SUBDEVICE_REGISTER_SERVICE_ID, subDeviceRegister_service_request_handler);
    serviceSiteManager->registerServiceRequestHandler(DOMAINID_REQUEST_SERVICE_ID, domainIdRequest_service_request_handler);
    serviceSiteManager->registerServiceRequestHandler(ENGINEER_REQUEST_SERVICE_ID, engineer_service_request_handler);
    serviceSiteManager->registerServiceRequestHandler(GETDEVICELIST_REQUEST_SERVICE_ID, getDeviceList_service_request_handler);
    serviceSiteManager->registerServiceRequestHandler(GETTVINFO_REQUEST_SERVICE_ID, getTvInfo_service_request_handler);
    serviceSiteManager->registerServiceRequestHandler(CONTROLDEVICE_REQUEST_SERVICE_ID, controlDevice_service_request_handler);
    serviceSiteManager->registerServiceRequestHandler(REDARREPORT_REQUEST_SERVICE_ID, radarReportEnable_service_request_handler);


    // 注册支持的消息ID， 有两个消息
    serviceSiteManager->registerMessageId(DEVICESTATUS_MESSAGE_ID);
    serviceSiteManager->registerMessageId(RADAR_MESSAGE_ID);
    serviceSiteManager->registerMessageId(MICPANELMESSAGE_ID);

    // 注册 Message 请求处理 handler
    serviceSiteManager->registerMessageHandler(DEVICESTATUS_MESSAGE_ID, deviceStatus_message_handler);
    serviceSiteManager->registerMessageHandler(RADAR_MESSAGE_ID, radar_message_handler);
    serviceSiteManager->registerMessageHandler(MICPANELMESSAGE_ID, micpanel_message_handler);

    // 创建线程, 启动服务器
    std::thread http_server_thread(http_server_thread_func);

    sleep(2);

    if (http_server_thread_end) {
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
            printf("启动 http 服务器线程错误.\n");
            break;
        }
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }

    http_server_thread.join();

    return -1;
}
