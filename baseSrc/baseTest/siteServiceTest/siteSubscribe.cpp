/*
 * 服务提供者示例程序
 *
 *  Created on: 2022年4月22日
 *      Author: van
 */

#include <thread>
#include <unistd.h>
#include <atomic>
#include <stdio.h>
#include "common/httplib.h"
#include "siteService/nlohmann/json.hpp"
#include "siteService/service_site_manager.h"
#include "log/Logging.h"
#include "qlibc/QData.h"


using namespace std;
using namespace servicesite;
using json = nlohmann::json;

// test_message_id_1 消息处理函数
void message_handler(const Request& request) {
    // 消息的json字符串位于request.body
    qlibc::QData data(request.body);
    string message_id = data.getString("message_id");
    if(message_id != "reportTracingTargets"){
        LOG_INFO << data.toJsonString();
    }
}

void message_handler1(const Request& request) {
    LOG_INFO << "hello";
}


// 标记线程错误
std::atomic<bool> http_server_thread_end(false);

void http_server_thread_func() {
    int code;

    ServiceSiteManager* serviceSiteManager = ServiceSiteManager::getInstance();

    // 启动服务器，参数为端口， 可用于单独的开发调试
    code = serviceSiteManager->start();

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
    serviceSiteManager->setServerPort(60002);

    // 注册 Message 请求处理 handler
//    serviceSiteManager->registerMessageHandler("scanResultMsg", message_handler);
//    serviceSiteManager->registerMessageHandler("singleDeviceBindSuccessMsg", message_handler);
//    serviceSiteManager->registerMessageHandler("bindEndMsg", message_handler);
//    serviceSiteManager->registerMessageHandler("singleDeviceUnbindSuccessMsg", message_handler);
//    serviceSiteManager->registerMessageHandler("device_state_changed", message_handler);
//    serviceSiteManager->registerMessageHandler("site_onoffline", message_handler);

    serviceSiteManager->registerMessageHandler("eventEnterArea", message_handler);
    serviceSiteManager->registerMessageHandler("eventLeaveArea", message_handler);
    serviceSiteManager->registerMessageHandler("reportTracingTargets", message_handler);



//    serviceSiteManager->registerMessageHandler("eventDining", message_handler);
//    serviceSiteManager->registerMessageHandler("eventCooking", message_handler);
//    serviceSiteManager->registerMessageHandler("eventSitAtDesk", message_handler);
//    serviceSiteManager->registerMessageHandler("eventSitAtSofa", message_handler);
//
//    serviceSiteManager->registerMessageHandler("eventFallDown", message_handler);




    // 订阅消息, 需要传入订阅站点的IP、端口号、消息ID列表
    int code;
    std::vector<string> messageIdList;
//    messageIdList.push_back("scanResultMsg");                   //扫描结果
//    messageIdList.push_back("singleDeviceBindSuccessMsg");      //单个绑定成功
//    messageIdList.push_back("bindEndMsg");                      //全部绑定结束
//    messageIdList.push_back("singleDeviceUnbindSuccessMsg");    //单个解绑成功
//    messageIdList.push_back("device_state_changed");            //设备状态变更
//    messageIdList.push_back("site_onoffline");                  //设备上下线


    messageIdList.push_back("eventEnterArea");
    messageIdList.push_back("eventLeaveArea");
    messageIdList.push_back("reportTracingTargets");
//    messageIdList.push_back("eventDining");
//    messageIdList.push_back("eventCooking");
//    messageIdList.push_back("eventSitAtDesk");
//    messageIdList.push_back("eventSitAtSofa");

//    messageIdList.push_back("eventFallDown");

    code = serviceSiteManager->subscribeMessage("192.168.58.119", 9003, messageIdList);
    if (code == ServiceSiteManager::RET_CODE_OK) {
        printf("subscribeMessage ok.\n");
    }else{
        printf("subscribeMessage failed now.\n");
    }

    // 创建线程, 启动服务器
    std::thread http_server_thread(http_server_thread_func);

    while (true) {
        sleep(1);

        if (http_server_thread_end) {
            printf("启动 http 服务器线程错误.\n");
            break;
        }
    }

    http_server_thread.join();

    return 0;
}
