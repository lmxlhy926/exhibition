//
// Created by 78472 on 2022/11/17.
//

#include <thread>
#include <unistd.h>
#include <atomic>
#include <cstdio>
#include <functional>

#include "qlibc/QData.h"
#include "param.h"
#include "log/Logging.h"
#include "service/serviceRequestHandler.h"
#include "siteService/nlohmann/json.hpp"
#include "siteService/service_site_manager.h"
#include "siteManage/siteManageUtil.h"

using namespace std;
using namespace httplib;
using json = nlohmann::json;
using namespace servicesite;


int main(int argc, char* argv[]){
    httplib::ThreadPool threadPool_(20);
    std::atomic<bool> http_server_thread_end(false);

    // 创建 serviceSiteManager 对象, 单例
    ServiceSiteManager* serviceSiteManager = ServiceSiteManager::getInstance();
    serviceSiteManager->setServerPort(QuerySitePort);
    serviceSiteManager->setSiteIdSummary(QuerySiteID, QuerySiteName);


    //获取单例对象
    SiteTree::getInstance();

    //注册本站点发布的消息
    serviceSiteManager->registerMessageId(Site_OnOffLine_MessageID);        //上下线消息
    serviceSiteManager->registerMessageId(Site_RegisterAgain_MessageID);    //重新注册消息
    serviceSiteManager->registerMessageId(Site_Requery_Result_MessageID);   //站点查询结果消息

    //站点注册服务
    serviceSiteManager->registerServiceRequestHandler(Site_Register_Service_ID,
                                                      [](const Request& request, Response& response) -> int{
        return site_register_service_handler(request, response);
    });

    //站点注销服务
    serviceSiteManager->registerServiceRequestHandler(Site_UnRegister_Service_ID,
                                                      [](const Request& request, Response& response) -> int{
        return site_unRegister_service_handler(request, response);
    });

    //站点查询服务
    serviceSiteManager->registerServiceRequestHandler(Site_Query_Service_ID,
                                                      [&threadPool_](const Request& request, Response& response) -> int{
        return site_query_service_handler(request, response, threadPool_);
    });


    //站点心跳服务
    serviceSiteManager->registerServiceRequestHandler(Site_Ping_Service_ID,
                                                      [](const Request& request, Response& response) -> int{
        return site_ping_service_handler(request, response);
    });


    //开启mdns服务器，接受查询
    threadPool_.enqueue([]{
        mdnsServiceStart();
    });


    //重新启动后，发布注册消息，使各个站点重新进行注册
    qlibc::QData registerAgain;
    registerAgain.setString("message_id", Site_RegisterAgain_MessageID);
    registerAgain.putData("content", qlibc::QData(Json::Value(Json::objectValue)));
    serviceSiteManager->publishMessage(Site_RegisterAgain_MessageID, registerAgain.toJsonString());
    LOG_PURPLE << "===>publish registerAgain message to notify all other sites to register again....";


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