
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
#include "service.h"
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
    serviceSiteManager->setServerPort(SynergySitePort);
    serviceSiteManager->setSiteIdSummary(SynergySiteID, SynergySiteName);

    //单例对象
  

//注册服务
    //
    // serviceSiteManager->registerServiceRequestHandler(Control_Service_ID,
    //                                                   [](const Request& request, Response& response) -> int{
    //     return synergy::cloudCommand_service_handler(request, response);
    // });

    // //语音控制服务
    // serviceSiteManager->registerServiceRequestHandler(VoiceControl_Service_ID,
    //                                                   [](const Request& request, Response& response) -> int{
    //     return synergy::voiceControl_service_handler(request, response);
    // });


    // //声明消息
    // serviceSiteManager->registerMessageId(Scene_Msg_MessageID);            //场景指令消息
    // serviceSiteManager->registerMessageId(ScanResultMsg);                  //扫描结果
    // serviceSiteManager->registerMessageId(SingleDeviceBindSuccessMsg);     //单个设备绑定结果
    


    //雷达点位消息
    servicesite::ServiceSiteManager::registerMessageHandler(ScanResultMsg,  radarMessageHandle);
   

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
