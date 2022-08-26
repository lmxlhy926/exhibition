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

using namespace std;
using namespace servicesite;
using json = nlohmann::json;

// test_message_id_1 消息处理函数
void message_handler(const Request& request) {
    // 消息的json字符串位于request.body
    auto message_json = json::parse(request.body);

    printf("received message:\n%s\n", message_json.dump(4).c_str());
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
    serviceSiteManager->registerMessageHandler("scanResultMsg", message_handler);
    serviceSiteManager->registerMessageHandler("singleDeviceBindSuccessMsg", message_handler);
    serviceSiteManager->registerMessageHandler("bindEndMsg", message_handler);
    serviceSiteManager->registerMessageHandler("singleDeviceUnbindSuccessMsg", message_handler);

    // 订阅消息, 需要传入订阅站点的IP、端口号、消息ID列表
    int code;
    std::vector<string> messageIdList;
    messageIdList.push_back("scanResultMsg");
    messageIdList.push_back("singleDeviceBindSuccessMsg");
    messageIdList.push_back("bindEndMsg");
    messageIdList.push_back("singleDeviceUnbindSuccessMsg");
    code = serviceSiteManager->subscribeMessage("127.0.0.1", 60009, messageIdList);
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
