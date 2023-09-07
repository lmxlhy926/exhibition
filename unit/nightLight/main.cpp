
#include <thread>
#include <unistd.h>
#include <atomic>
#include <cstdio>
#include <functional>

#include "param.h"
#include "qlibc/FileUtils.h"
#include "log/Logging.h"
#include "common/httpUtil.h"
#include "source/lightManage.h"
#include "service/service.h"
#include "siteService/nlohmann/json.hpp"
#include "siteService/service_site_manager.h"

using namespace std;
using namespace servicesite;
using namespace httplib;
using namespace std::placeholders;
using json = nlohmann::json;

int main(int argc, char* argv[]) {
    //增加log打印
    string path = "/data/changhong/edge_midware/lhy/nightLightSiteLog.txt";
    muduo::logInitLogger(path);

    LOG_RED << "-----------------------------------------";
    LOG_RED << "-----------------------------------------";
    LOG_RED << "---------------LIGHTNIGHT START-------------";
    LOG_RED << "-----------------------------------------";
    LOG_RED << "-----------------------------------------";

    httplib::ThreadPool threadPool_(5);

    // 创建 serviceSiteManager 对象, 单例
    ServiceSiteManager* serviceSiteManager = ServiceSiteManager::getInstance();
    serviceSiteManager->setServerPort(LightFlowSitePort);
    serviceSiteManager->setSiteIdSummary(LightFlowSiteID, LightFlowSiteName);

    //单例对象
    lightManage::getInstance();

    //保存灯带
    serviceSiteManager->registerServiceRequestHandler(SaveStrip_Service_ID,
                                                      [](const Request& request, Response& response) -> int{
        return saveStrip_service_request_handler(request, response);
    });

    //删除灯带
    serviceSiteManager->registerServiceRequestHandler(DelStrip_Service_ID,
                                                      [](const Request& request, Response& response) -> int{
        return delStrip_service_request_handler(request, response);
    });

    //获取灯带列表
    serviceSiteManager->registerServiceRequestHandler(GetStripList_Service_ID,
                                                      [](const Request& request, Response& response) -> int{
        return getStripList_service_request_handler(request, response);
    });


    // //声明消息
    // serviceSiteManager->registerMessageId(Scene_Msg_MessageID);            //场景指令消息
    

 
    //雷达点位消息
    servicesite::ServiceSiteManager::registerMessageHandler(Radar_Msg_MessageID,  radarMessageHandle);
    threadPool_.enqueue([&](){
        while(true){
            int code;
            std::vector<string> messageIdList;
            messageIdList.push_back(Radar_Msg_MessageID);
            code = serviceSiteManager->subscribeMessage("127.0.0.1", 9011, messageIdList);
            if (code == ServiceSiteManager::RET_CODE_OK) {
                printf("subscribeMessage radarPoints ok.\n");
                break;
            }
            std::this_thread::sleep_for(std::chrono::seconds(10));
            LOG_RED << "subscribeMessage radarPoints failed....., start to subscribe in 10 seconds";
        }
    });
   

    // 站点监听线程启动
    while(true){
        //自启动方式
        // int code = serviceSiteManager->start();
        //注册启动方式
        int code = serviceSiteManager->startByRegister();
        if(code != 0){
            std::cout << "===>lightNightSite startByRegister error, code = " << code << std::endl;
            std::cout << "===>lightNightSite startByRegister in 3 seconds...." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(3));
        }
    }

    return 0;
}
