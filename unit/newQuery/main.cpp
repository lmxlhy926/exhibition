//
// Created by 78472 on 2022/11/17.
//

#include "mdns/mdnsUtil.h"

#include <thread>
#include <unistd.h>
#include <atomic>
#include <cstdio>
#include <functional>

#include "param.h"
#include "service/serviceRequestHandler.h"
#include "siteService/nlohmann/json.hpp"
#include "siteService/service_site_manager.h"

using namespace std;
using namespace httplib;
using json = nlohmann::json;
using namespace servicesite;
using namespace std::placeholders;




int main(int argc, char* argv[]){
    httplib::ThreadPool threadPool_(30);
    std::atomic<bool> http_server_thread_end(false);

    // 创建 serviceSiteManager 对象, 单例
    ServiceSiteManager* serviceSiteManager = ServiceSiteManager::getInstance();
    serviceSiteManager->setServerPort(QuerySitePort);
    serviceSiteManager->setSiteIdSummary(QuerySiteID, QuerySiteName);


    serviceSiteManager->registerMessageId("");   //场景指令消息

    //设备控制
    serviceSiteManager->registerServiceRequestHandler(Site_Query_Service_ID,
                                                      [](const Request& request, Response& response) -> int{
        return site_discovery_service_handler(request, response);
    });

    threadPool_.enqueue([]{
        mdnsServiceStart();
    });

//    serviceSiteManager->registerMessageHandler(Site_OnOffLine_MessageID, [](const Request& request){
//        //每次站点上线都会触发重新获取设备列表、组列表
//        qlibc::QData data(request.body);
//        string site_status = data.getData("content").getString("site_status");
//        if(site_status == "online"){    //站点上线时，重新获取列表
//            DeviceManager::getInstance()->listChanged();
//            GroupManager::getInstance()->listChanged();
//        }
//    });

//    threadPool_.enqueue([&](){
//        while(true){
//            int code;
//            std::vector<string> messageIdList;
//            messageIdList.push_back(Site_OnOffLine_MessageID);
//            code = serviceSiteManager->subscribeMessage(RequestIp, QuerySitePort, messageIdList);
//            if (code == ServiceSiteManager::RET_CODE_OK) {
//                printf("subscribeMessage Site_OnOffLine_MessageID ok....\n");
//                break;
//            }
//
//            std::this_thread::sleep_for(std::chrono::seconds(3));
//            printf("subscribed Site_OnOffLine_MessageID failed....., start to subscribe in 3 seconds\n");
//        }
//    });


    // 站点监听线程启动
    threadPool_.enqueue([&](){
        while(true){
            //自启动方式
            int code = serviceSiteManager->start();
            //注册启动方式
//            int code = serviceSiteManager->startByRegister();
            if(code != 0){
                std::cout << "===>querySite startByRegister error, code = " << code << std::endl;
                std::cout << "===>querySite startByRegister in 3 seconds...." << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(3));
            }else{
                std::cout << "===>querySite startByRegister successfully....." << std::endl;
                break;
            }
        }
    });

    while(true){
        std::this_thread::sleep_for(std::chrono::seconds(60 *10));
    }

    return 0;




}