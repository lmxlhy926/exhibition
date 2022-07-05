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
#include <fstream>
#include "qlibc/QData.h"

using namespace std;
using namespace servicesite;

using json = nlohmann::json;

const string TEST_SITE_ID_1 = "publish";
const string TEST_SITE_NAME_1 = "publish";

const string TEST_MESSAGE_ID_1 = "eventLeaveHome";

void publish_message(void){
    qlibc::QData data;
    data.loadFromFile(R"(D:\bywg\project\exhibition\baseSrc\siteService\test\publish.json)");

    ServiceSiteManager* serviceSiteManager = ServiceSiteManager::getInstance();

    // 把要发布的消息 json 字符串传入即可， 由库来向订阅过的站点发送消息
    serviceSiteManager->publishMessage(TEST_MESSAGE_ID_1, data.toJsonString());
    std::cout << "---publish---" << std::endl;

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

    serviceSiteManager->setServerPort(60003);

    // 创建线程, 启动服务器
    std::thread http_server_thread(http_server_thread_func);

    int code;

    while (true) {
        sleep(3);

        if (http_server_thread_end) {
            printf("启动 http 服务器线程错误.\n");
            break;
        }

        // 发布消息
        publish_message();
    }

    http_server_thread.join();

    return 0;
}
