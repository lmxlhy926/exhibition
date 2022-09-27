
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
#include "deviceManager.h"

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

    //单例对象
    DeviceManager::getInstance();

    //站点请求管理
    SiteRecord::getInstance()->addSite(BleSiteID, RequestIp, BleSitePort);
    SiteRecord::getInstance()->addSite(ZigbeeSiteID, RequestIp, ZigbeeSitePort);
    SiteRecord::getInstance()->addSite(TvAdapterSiteID, RequestIp, TvAdapterSitePort);
    SiteRecord::getInstance()->addSite(SceneSiteID, RequestIp, SceneSitePort);

    //设备控制
    serviceSiteManager->registerServiceRequestHandler(Control_Device_Service_ID,
                                                      [](const Request& request, Response& response) -> int{
        return synergy::device_control_service_handler(request, response);
    });

    //场景接口
    serviceSiteManager->registerServiceRequestHandler(SceneCommand_Service_ID,
                                                      [](const Request& request, Response& response) -> int{
        return synergy::sceneCommand_service_handler(request, response);
    });

    //获取设备列表
    serviceSiteManager->registerServiceRequestHandler(GetDeviceList_Service_ID,
                                                      [](const Request& request, Response& response) -> int{
        return synergy::getDeviceList_service_handler(request, response);
    });

    //获取分组列表
    serviceSiteManager->registerServiceRequestHandler(GetGroupList_Service_ID,
                                                      [](const Request& request, Response& response) -> int{
        return synergy::getGroupList_service_handler(request, response);
    });


    //注册messageID对应的handler;
    serviceSiteManager->registerMessageHandler(DeviceList_changed_messageID, [](const Request& request){
        DeviceManager::getInstance()->listChanged();
    });

//    threadPool_.enqueue([&](){
//        while(true){
//            int code1, code2, code3;
//            std::vector<string> messageIdList;
//            messageIdList.push_back(DeviceList_changed_messageID);
//            code1 = serviceSiteManager->subscribeMessage(RequestIp, BleSitePort, messageIdList);
//            code2 = serviceSiteManager->subscribeMessage(RequestIp, ZigbeeSitePort, messageIdList);
//            code3 = serviceSiteManager->subscribeMessage(RequestIp, TvAdapterSitePort, messageIdList);
//
//            if (code1 == ServiceSiteManager::RET_CODE_OK && code2 == ServiceSiteManager::RET_CODE_OK && code3 == ServiceSiteManager::RET_CODE_OK) {
//                printf("subscribeMessage DeviceList_changed_messageID ok....\n");
//                break;
//            }
//
//            std::this_thread::sleep_for(std::chrono::seconds(3));
//            printf("subscribed DeviceList_changed_messageID failed....., start to subscribe in 3 seconds\n");
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
