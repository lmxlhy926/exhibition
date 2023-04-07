
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
#include "sourceManage/deviceManager.h"
#include "sourceManage/groupManager.h"
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
    siteManager::getInstance();

    //站点请求管理
    serviceSiteManager->registerMessageId(Scene_Msg_MessageID);   //场景指令消息


//注册服务
    //设备控制 + 场景命令
    serviceSiteManager->registerServiceRequestHandler(Control_Service_ID,
                                                      [](const Request& request, Response& response) -> int{
        return synergy::cloudCommand_service_handler(request, response);
    });

    //语音控制服务
    serviceSiteManager->registerServiceRequestHandler(VoiceControl_Service_ID,
                                                      [](const Request& request, Response& response) -> int{
        return synergy::voiceControl_service_handler(request, response);
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

    //请求设备列表更新
    serviceSiteManager->registerServiceRequestHandler(UpdateDeviceList_Service_ID,
                                                      [](const Request& request, Response& response)->int{
        return synergy::updateDeviceList_service_handler(request, response);
    });

    //请求组列表更新
    serviceSiteManager->registerServiceRequestHandler(UpdateGroupList_Service_ID,
                                                      [](const Request& request, Response& response)->int{
        return synergy::updateGroupList_service_handler(request, response);
    });

    //获取面板列表
    serviceSiteManager->registerServiceRequestHandler(GetPanelList_Service_ID, synergy::getPanelList_service_handler);



    //注册重置网关回调
    serviceSiteManager->registerServiceRequestHandler(Reset_Device_Service_ID,
                                                      [](const Request& request, Response& response) -> int{
        return synergy::reset_device_service_handler(request, response);
    });

    //注册设备扫描回调
    serviceSiteManager->registerServiceRequestHandler(Scan_Device_Service_ID,
                                                      [](const Request& request, Response& response) -> int{
        return synergy::scan_device_service_handler(request, response);
    });

    //注册设备绑定回调
    serviceSiteManager->registerServiceRequestHandler(Add_Device_Service_ID,
                                                      [](const Request& request, Response& response) -> int{
        return synergy::add_device_service_handler(request, response);
    });

    //注册设备解绑回调
    serviceSiteManager->registerServiceRequestHandler(Del_Device_Service_ID,
                                                      [](const Request& request, Response& response) -> int{
        return synergy::del_device_service_handler(request, response);
    });

    //注册设备属性修改回调
    serviceSiteManager->registerServiceRequestHandler(ConfigProperty_Service_ID,
                                                      [](const Request& request, Response& response) -> int{
        return synergy::configProperty_device_service_handler(request, response);
    });


    //注册设备控制回调
    serviceSiteManager->registerServiceRequestHandler(Control_Device_Service_ID,
                                                      [](const Request& request, Response& response) -> int{
        return synergy::deviceControl_service_handler(request, response);
    });


    //获取设备列表
    serviceSiteManager->registerServiceRequestHandler(Get_DeviceList_Service_ID,
                                                      [](const Request& request, Response& response) -> int{
        return synergy::getDeviceList_service_handler(request, response);
    });

    //获取设备状态
    serviceSiteManager->registerServiceRequestHandler(Get_DeviceState_Service_ID,
                                                      [](const Request& request, Response& response) -> int{
        return synergy::get_device_state_service_handler(request, response);
    });


    //创建分组
    serviceSiteManager->registerServiceRequestHandler(CreateGroup_Device_Service_ID, synergy::create_group_service_handler);

    //删除分组
    serviceSiteManager->registerServiceRequestHandler(DeleteGroup_Device_Service_ID,
                                                      [](const Request& request, Response& response) -> int{
        return synergy::delete_group_service_handler(request, response);
    });

    //重命名分组
    serviceSiteManager->registerServiceRequestHandler(RenameGroup_Device_Service_ID, synergy::rename_group_service_handler);

    //添加设备进分组
    serviceSiteManager->registerServiceRequestHandler(AddDevice2Group_Device_Service_ID,
                                                      [](const Request& request, Response& response) -> int{
        return synergy::addDevice2Group_service_handler(request, response);
    });


    //从分组删除设备
    serviceSiteManager->registerServiceRequestHandler(RemoveDeviceFromGroup_Device_Service_ID,
                                                      [](const Request& request, Response& response) -> int{
        return synergy::removeDeviceFromGroup_service_handler(request, response);
    });

    //控制分组
    serviceSiteManager->registerServiceRequestHandler(ControlGroup_Device_Service_ID,
                                                      [](const Request& request, Response& response) -> int{
        return synergy::groupControl_service_handler(request, response);
    });

    //获取分组列表
    serviceSiteManager->registerServiceRequestHandler(GetGroupList_Device_Service_ID, synergy::getGroupList_service_handler);



    //声明消息
    serviceSiteManager->registerMessageId(ScanResultMsg);                  //扫描结果
    serviceSiteManager->registerMessageId(SingleDeviceBindSuccessMsg);     //单个设备绑定结果
    serviceSiteManager->registerMessageId(SingleDeviceUnbindSuccessMsg);   //单个设备解绑结果
    serviceSiteManager->registerMessageId(BindEndMsg);                     //绑定结束
    serviceSiteManager->registerMessageId(Device_State_Changed);           //设备状态改变

    //注册消息处理函数
    servicesite::ServiceSiteManager::registerMessageHandler(ScanResultMsg,                 synergy::messagePublish);
    servicesite::ServiceSiteManager::registerMessageHandler(SingleDeviceBindSuccessMsg,    synergy::messagePublish);
    servicesite::ServiceSiteManager::registerMessageHandler(SingleDeviceUnbindSuccessMsg,  synergy::messagePublish);
    servicesite::ServiceSiteManager::registerMessageHandler(BindEndMsg,                    synergy::messagePublish);
    servicesite::ServiceSiteManager::registerMessageHandler(Device_State_Changed,          synergy::messagePublish);


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
