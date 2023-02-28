
#include <thread>
#include <unistd.h>
#include <atomic>
#include <cstdio>
#include <functional>

#include "siteService/nlohmann/json.hpp"
#include "siteService/service_site_manager.h"
#include "qlibc/FileUtils.h"
#include "service/serviceRequestHandler.h"
#include "param.h"
#include "common/httpUtil.h"
#include "deviceGroupManage/deviceManager.h"
#include "deviceGroupManage/groupManager.h"
#include "log/Logging.h"

using namespace std;
using namespace servicesite;
using namespace httplib;
using namespace std::placeholders;
using json = nlohmann::json;

int main(int argc, char* argv[]) {
    //增加log打印
    string path = "/data/changhong/edge_midware/lhy/synergySiteLog.txt";
    muduo::logInitLogger(path);

    httplib::ThreadPool threadPool_(5);

    // 创建 serviceSiteManager 对象, 单例
    ServiceSiteManager* serviceSiteManager = ServiceSiteManager::getInstance();
    serviceSiteManager->setServerPort(SynergySitePort);
    serviceSiteManager->setSiteIdSummary(SynergySiteID, SynergySiteName);

    //单例对象
    DeviceManager::getInstance();
    GroupManager::getInstance();

    //站点请求管理
    SiteRecord::getInstance()->addSite(SceneSiteID, RequestIp, SceneSitePort);  //加入场景站点

    serviceSiteManager->registerMessageId(Scene_Msg_MessageID);   //场景指令消息

    //设备控制 + 场景命令
    serviceSiteManager->registerServiceRequestHandler(Control_Service_ID,
                                                      [](const Request& request, Response& response) -> int{
        return synergy::cloudCommand_service_handler(request, response);
    });

    //获取设备列表：蓝牙，zigbee, tv_adapter
    serviceSiteManager->registerServiceRequestHandler(GetDeviceList_Service_ID,
                                                      [](const Request& request, Response& response) -> int{
        return synergy::getDeviceList_service_handler(request, response);
    });

    //获取分组列表: 蓝牙，zigbee, tv_adapter
    serviceSiteManager->registerServiceRequestHandler(GetGroupList_Service_ID,
                                                      [](const Request& request, Response& response) -> int{
        return synergy::getGroupList_service_handler(request, response);
    });

    //语音控制服务
    serviceSiteManager->registerServiceRequestHandler(VoiceControl_Service_ID,
                                                      [](const Request& request, Response& response) -> int{
        return synergy::voiceControl_service_handler(request, response);
    });

    //设备控制服务
    serviceSiteManager->registerServiceRequestHandler(DeviceControl_Service_ID,
                                                      [](const Request& request, Response& response) -> int{
        return synergy::deviceControl_service_handler(request, response);
    });

    //分组控制服务
    serviceSiteManager->registerServiceRequestHandler(GroupControl_Service_ID,
                                                      [](const Request& request, Response& response) -> int{
        return synergy::groupControl_service_handler(request, response);
    });

    //蓝牙设备注册服务
    serviceSiteManager->registerServiceRequestHandler(BleGroupRegister_Service_ID,
                                                      [](const Request& request, Response& response)->int{
        return synergy::bleDeviceRegister_service_handler(request, response);
    });

    //蓝牙设备操作
    serviceSiteManager->registerServiceRequestHandler(BleDeviceOperation_Service_ID,
                                                      [](const Request& request, Response& response)->int{
        return synergy::bleDeviceOperation_service_handler(request, response);
    });


#if 0
    serviceSiteManager->registerMessageHandler(Site_OnOffLine_MessageID, [](const Request& request){
        //每次站点上线都会触发重新获取设备列表、组列表
        qlibc::QData data(request.body);
        string site_status = data.getData("content").getString("site_status");
        if(site_status == "online"){    //站点上线时，重新获取列表
            DeviceManager::getInstance()->listChanged();
            GroupManager::getInstance()->listChanged();
        }
    });

    threadPool_.enqueue([&](){
        while(true){
            int code;
            std::vector<string> messageIdList;
            messageIdList.push_back(Site_OnOffLine_MessageID);
            code = serviceSiteManager->subscribeMessage(RequestIp, QuerySitePort, messageIdList);
            if (code == ServiceSiteManager::RET_CODE_OK) {
                printf("subscribeMessage Site_OnOffLine_MessageID ok....\n");
                break;
            }

            std::this_thread::sleep_for(std::chrono::seconds(3));
            printf("subscribed Site_OnOffLine_MessageID failed....., start to subscribe in 3 seconds\n");
        }
    });
#endif

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
