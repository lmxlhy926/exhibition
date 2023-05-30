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
#include <fstream>

#include "http/httplib.h"
#include "siteService/nlohmann/json.hpp"
#include "siteService/service_site_manager.h"
#include "qlibc/QData.h"
#include "log/Logging.h"


using namespace std;
using namespace servicesite;
using json = nlohmann::json;

static const string CONFIG_SITE_ID = "test";
static const string CONFIG_SITE_ID_NAME = "测试站点";
const string TEST_MESSAGE_ID = "register2QuerySiteAgain";


void publish_message(void){
    qlibc::QData data;
    data.setString("message_id", "register2QuerySiteAgain");
    data.putData("content", qlibc::QData());
    ServiceSiteManager* serviceSiteManager = ServiceSiteManager::getInstance();
    serviceSiteManager->publishMessage(TEST_MESSAGE_ID, data.toJsonString());
    std::cout << "---publish---" << std::endl;
}

int main(int argc, char* argv[]) {

    //. 设置线程池
    httplib::ThreadPool threadPool_(10);
    std::atomic<bool> http_server_thread_end(false);

    //. 配置本站点启动信息
    ServiceSiteManager* serviceSiteManager = ServiceSiteManager::getInstance();
    serviceSiteManager->setServerPort(60003);
    serviceSiteManager->setSiteIdSummary(CONFIG_SITE_ID, CONFIG_SITE_ID_NAME);

    serviceSiteManager->registerMessageId(TEST_MESSAGE_ID);

    serviceSiteManager->registerMessageHandler( "register2QuerySiteAgain", [](const Request& request){
        LOG_INFO << "received: " << request.body;
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

    //订阅自己
    threadPool_.enqueue([&](){
        while(true){
            std::vector<string> messageIdList{
                    "register2QuerySiteAgain",
            };
            int code = serviceSiteManager->subscribeMessage("172.29.90.147", 60003, messageIdList);
            if (code == ServiceSiteManager::RET_CODE_OK) {
                LOG_INFO << "subscribeMessage ok.";
                break;
            }
            std::this_thread::sleep_for(std::chrono::seconds(3));
            LOG_INFO << "subscribed  failed....., start to subscribe in 3 seconds";
        }
    });

    threadPool_.enqueue([&](){
        while(true){
            sleep(10);
            publish_message();
        }
    });


    while(true){
        std::this_thread::sleep_for(std::chrono::seconds(30));
    }

    return 0;
}

