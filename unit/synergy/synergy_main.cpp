
#include <thread>
#include <unistd.h>
#include <atomic>
#include <cstdio>
#include <functional>

#include "siteService/nlohmann/json.hpp"
#include "siteService/service_site_manager.h"
#include "qlibc/FileUtils.h"
#include "serviceRequestHandler.h"
#include "param.h"
#include "common/httpUtil.h"

using namespace std;
using namespace servicesite;
using namespace httplib;
using namespace std::placeholders;
using json = nlohmann::json;

int main(int argc, char* argv[]) {

    httplib::ThreadPool threadPool_(30);
    std::atomic<bool> http_server_thread_end(false);

    // 创建 serviceSiteManager 对象, 单例
    ServiceSiteManager* serviceSiteManager = ServiceSiteManager::getInstance();
    serviceSiteManager->setServerPort(SynergySitePort);
    serviceSiteManager->setSiteIdSummary(SynergySiteID, SynergySiteName);

    //站点请求管理
    SiteRecord::getInstance()->addSite(BleSiteID, RequestIp, BleSitePort);
    SiteRecord::getInstance()->addSite(ZigbeeSiteID, RequestIp, ZigbeeSitePort);
    SiteRecord::getInstance()->addSite(TvAdapterSiteID, RequestIp, TvAdapterSitePort);


    // 注册 Service 请求处理 handler
    serviceSiteManager->registerServiceRequestHandler(Control_Device_Service_ID,
                                                      [&](const Request& request, Response& response) -> int{
        return device_control_service_handler(request, response);
                                                      });

    //注册messageID对应的handler;
//    serviceSiteManager->registerMessageHandler(REGISTERAGAIN_MESSAGE_ID, synergy::register2QuerySite);
//
//    threadPool_.enqueue([&](){
//        while(true){
//            int code;
//            std::vector<string> messageIdList;
//            messageIdList.push_back(REGISTERAGAIN_MESSAGE_ID);
//            code = serviceSiteManager->subscribeMessage(RequestIp, QuerySitePort, messageIdList);
//
//            if (code == ServiceSiteManager::RET_CODE_OK) {
//                printf("subscribeMessage REGISTERAGAIN_MESSAGE_ID ok.\n");
//                break;
//            }
//
//            std::this_thread::sleep_for(std::chrono::seconds(3));
//            printf("subscribed REGISTERAGAIN_MESSAGE_ID failed....., start to subscribe in 3 seconds\n");
//        }
//    });


    // 站点监听线程启动
    threadPool_.enqueue([&](){
        while(true){
            //自启动方式
//            int code = serviceSiteManager->start();
            //注册启动方式
            int code = serviceSiteManager->startByRegister();
            if(code != 0){
                std::cout << "===>synergySite startByRegister error, code = " << code << std::endl;
                std::cout << "===>synergySite startByRegister in 3 seconds...." << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(3));
            }else{
                std::cout << "===>synergySite startByRegister successfully....." << std::endl;
                break;
            }
        }
    });

    while(true){
        std::this_thread::sleep_for(std::chrono::seconds(60 *10));
    }

    return 0;
}
