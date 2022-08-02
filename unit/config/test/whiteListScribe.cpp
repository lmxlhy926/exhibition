//
// Created by 78472 on 2022/7/14.
//

#include <thread>
#include <atomic>
#include <cstdio>

#include "siteService/nlohmann/json.hpp"
#include "siteService/service_site_manager.h"
#include "qlibc/FileUtils.h"
#include "log/Logging.h"
#include "qlibc/QData.h"

using namespace std;
using namespace servicesite;
using namespace httplib;
using namespace std::placeholders;
using json = nlohmann::json;

static const string CONFIG_SITE_ID = "test";
static const string CONFIG_SITE_ID_NAME = "测试站点";


void message_handler(const Request& request) {
    // 消息的json字符串位于request.body
    auto message_json = json::parse(request.body);
    qlibc::QData data(request.body);

    LOG_YELLOW << "qlibc---->" << data.toJsonString();
    LOG_YELLOW << ".............................................................................";

    LOG_PURPLE << "message_json.dump(4).c_str()------>" << message_json.dump(4).c_str();
    LOG_PURPLE << ".............................................................................";

    LOG_GREEN << "request.body------------->" << request.body;
    LOG_GREEN << ".............................................................................";
}


int main(int argc, char* argv[]) {

    //. 设置线程池
    httplib::ThreadPool threadPool_(10);
    std::atomic<bool> http_server_thread_end(false);

    //. 配置本站点启动信息
    ServiceSiteManager* serviceSiteManager = ServiceSiteManager::getInstance();
    serviceSiteManager->setServerPort(9999);
    serviceSiteManager->setSiteIdSummary(CONFIG_SITE_ID, CONFIG_SITE_ID_NAME);


    //注册订阅消息messageID;
    serviceSiteManager->registerMessageId("whiteList");
    serviceSiteManager->registerMessageId("receivedWhiteList");

    //注册messageID对应的handler;
    serviceSiteManager->registerMessageHandler("whiteList", message_handler);
    serviceSiteManager->registerMessageHandler("receivedWhiteList", message_handler);

    threadPool_.enqueue([&](){
        while(true){
            int code;
            std::vector<string> messageIdList;
            messageIdList.push_back("whiteList");
            messageIdList.push_back("receivedWhiteList");
            code = serviceSiteManager->subscribeMessage("127.0.0.1", 9006, messageIdList);

            if (code == ServiceSiteManager::RET_CODE_OK) {
                LOG_PURPLE << "subscribeMessage ok......";
                break;
            }

            std::this_thread::sleep_for(std::chrono::seconds(3));
            LOG_RED << "subscribed failed....., start to subscribe in 3 seconds";
        }
    });

    // 站点监听线程启动
    threadPool_.enqueue([&](){
        while(true){
            //自启动方式
            int code = serviceSiteManager->start();
            //注册启动方式
//            int code = serviceSiteManager->startByRegister();
            if(code != 0){
                LOG_RED << "===>test_site startByRegister error, code = " << code;
                LOG_RED << "===>test_site startByRegister in 3 seconds....";
                std::this_thread::sleep_for(std::chrono::seconds(3));
            }else{
                LOG_RED << "===>test_site startByRegister successfully.....";
                break;
            }
        }
    });

    while(true){
        std::this_thread::sleep_for(std::chrono::seconds(60 *10));
    }

    return 0;
}
