//
// Created by 78472 on 2022/5/17.
//

#include <thread>
#include <unistd.h>
#include <atomic>
#include <cstdio>
#include <functional>

#include "siteService/nlohmann/json.hpp"
#include "siteService/service_site_manager.h"

#include "socket/httplib.h"
#include "socket/socketServer.h"
#include "common/configParamUtil.h"
#include "qlibc/FileUtils.h"
#include "mqtt/mqttClient.h"
#include "serviceHandler.h"

using namespace std;
using namespace servicesite;
using namespace httplib;
using namespace std::placeholders;
using json = nlohmann::json;

static const string CONFIG_SITE_ID = "测试站点";
static const string CONFIG_SITE_ID_NAME = "测试站点";


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

    httplib::ThreadPool threadPool_(30);
    std::atomic<bool> http_server_thread_end(false);

    // 创建 serviceSiteManager 对象, 单例
    ServiceSiteManager* serviceSiteManager = ServiceSiteManager::getInstance();
    serviceSiteManager->setServerPort(60003);


    //注册获取设备列表函数
    serviceSiteManager->registerServiceRequestHandler(DEVICE_LIST_REQUEST_SERVICE_ID,deviceList_service_request_handler);
    //注册子设备注册处理函数
    serviceSiteManager->registerServiceRequestHandler(CONTROL_DEVICE_REGISTER_SERVICE_ID,controlDevice_service_request_handler);
    //注册获取tvMac处理函数
    serviceSiteManager->registerServiceRequestHandler(TVMAC_SERVICE_ID,tvMac_service_request_handler);


    // 站点监听线程启动
    threadPool_.enqueue([&](){
        // 启动服务器，参数为端口， 可用于单独的开发调试
        int code = serviceSiteManager->start();

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
