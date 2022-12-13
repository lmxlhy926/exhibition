
#include <thread>
#include "parameter.h"
#include "service/serviceRequestHandler.h"
#include "common/httpUtil.h"
#include "serial/telinkDongle.h"
#include "formatTrans/bleConfig.h"
#include "formatTrans/recvPackageParse.h"
#include "formatTrans/statusEvent.h"
#include "logic/scanListmanage.h"
#include "logic/logicControl.h"
#include "siteService/nlohmann/json.hpp"
#include "siteService/service_site_manager.h"

using namespace std;
using namespace servicesite;
using namespace httplib;
using json = nlohmann::json;

int main(int argc, char* argv[]) {

    httplib::ThreadPool threadPool_(10);
    //站点请求管理
    SiteRecord::getInstance()->addSite(ConfigSiteName, LocalIp, ConfigPort);

    // 创建 serviceSiteManager 对象, 单例
    ServiceSiteManager* serviceSiteManager = ServiceSiteManager::getInstance();
    serviceSiteManager->setServerPort(BLE_SITE_PORT);
    ServiceSiteManager::setSiteIdSummary(BLE_SITE_ID, BLE_SITE_ID_NAME);

    //设置配置文件加载路径, 加载配置文件
    bleConfig* configPathPtr = bleConfig::getInstance();
    configPathPtr->setConfigPath(string(argv[1]));

    if(argc >= 3 && string(argv[2]) == "disableUpload"){
        LOG_INFO << "disableUpload...........";
        RecvPackageParse::disableUpload();
    }

    //单例对象
    ScanListmanage::getInstance();

    //注册串口上报回调函数，初始化并打开串口
    SerialParamStruct serialParam;
    serialParam.stopbits = 1;
    serialParam.databits = 8;
    serialParam.parity = 'N';
    serialParam.baudrate = 115200;

    //获取串口号、注册回调函数、循环直到打开串口
    string serialName = bleConfig::getInstance()->getSerialData().getString("serial");
    TelinkDongle* telinkDonglePtr = TelinkDongle::getInstance(serialName);
    telinkDonglePtr->registerPkgMsgFunc(RecvPackageParse::handlePackageString);
    while(true){
        if(telinkDonglePtr->openSerial(serialParam)){
            telinkDonglePtr->startReceive();
            LOG_PURPLE << "===>startDongle successfully, serialName <" << serialName << ">.......";
            break;
        }else{
            LOG_RED << "==>failed to open the serial<" << serialName << ">....";
            std::this_thread::sleep_for(std::chrono::seconds(3));
        }
    }

    //创建控制类，传递给注册的回调函数
    LogicControl lc;

    //注册本站点支持的消息
    serviceSiteManager->registerMessageId(ScanResultMsg);                  //扫描结果
    serviceSiteManager->registerMessageId(SingleDeviceBindSuccessMsg);     //单个设备绑定结果
    serviceSiteManager->registerMessageId(SingleDeviceUnbindSuccessMsg);   //单个设备解绑结果
    serviceSiteManager->registerMessageId(BindEndMsg);                     //绑定结束
    serviceSiteManager->registerMessageId(Device_State_Changed);           //设备状态改变


    //注册设备扫描回调
    serviceSiteManager->registerServiceRequestHandler(Scan_Device_Service_ID,
                                                      [&lc](const Request& request, Response& response) -> int{
        return scan_device_service_handler(request, response, lc);
    });

    //注册设备绑定回调
    serviceSiteManager->registerServiceRequestHandler(Add_Device_Service_ID,
                                                      [&lc](const Request& request, Response& response) -> int{
        return add_device_service_handler(request, response, lc);
    });

    //注册设备解绑回调
    serviceSiteManager->registerServiceRequestHandler(Del_Device_Service_ID,
                                                      [&lc](const Request& request, Response& response) -> int{
        return del_device_service_handler(request, response, lc);
    });

    //注册设备控制回调
    serviceSiteManager->registerServiceRequestHandler(Control_Device_Service_ID,
                                                      [&lc](const Request& request, Response& response) -> int{
        return control_device_service_handler(request, response, lc);
    });
    
    //修改设备信息
    serviceSiteManager->registerServiceRequestHandler(Config_Device_Property_Service_ID,
                                                      [](const Request& request, Response& response) -> int{
        return device_config_service_handler(request, response);
    });

    //获取设备列表
    serviceSiteManager->registerServiceRequestHandler(Get_DeviceList_Service_ID,
                                                      [](const Request& request, Response& response) -> int{
        return get_device_list_service_handler(request, response);
    });

    //依据房间名获取设备列表
    serviceSiteManager->registerServiceRequestHandler(Get_DeviceList_byRoomName_Service_ID,
                                                      [](const Request& request, Response& response) -> int{
        return get_device_list_byRoomName_service_handler(request, response);
    });

    //存储设备列表
    serviceSiteManager->registerServiceRequestHandler(Save_DeviceList_Service_ID,
                                                      [](const Request& request, Response& response) -> int{
        return save_deviceList_service_handler(request, response);
    });


    //获取设备状态
    serviceSiteManager->registerServiceRequestHandler(Get_DeviceState_Service_ID,
                                                      [](const Request& request, Response& response) -> int{
        return get_device_state_service_handler(request, response);
    });

    //接收二进制单步指令，用于单步指令调试
    serviceSiteManager->registerServiceRequestHandler(Ble_Device_Test_Command_Service_ID,
                                                      [](const Request& request, Response& response) -> int{
        return BleDevice_command_test_service_handler(request, response);
    });


    //创建分组
    serviceSiteManager->registerServiceRequestHandler(CreateGroup_Device_Service_ID, create_group_service_handler);

    //删除分组
    serviceSiteManager->registerServiceRequestHandler(DeleteGroup_Device_Service_ID,
                                                      [&lc](const Request& request, Response& response) -> int{
        return delete_group_service_handler(request, response, lc);
    });

    //重命名分组
    serviceSiteManager->registerServiceRequestHandler(RenameGroup_Device_Service_ID, rename_group_service_handler);

    //添加设备进分组
    serviceSiteManager->registerServiceRequestHandler(AddDevice2Group_Device_Service_ID,
                                                      [&lc](const Request& request, Response& response) -> int{
        return addDevice2Group_service_handler(request, response, lc);
    });

    //按照指定房间进行分组
    serviceSiteManager->registerServiceRequestHandler(AddDevice2GroupByRoom_Device_Service_ID,
                                                      [&lc](const Request& request, Response& response) -> int{
        return groupByRoomname_service_handler(request, response, lc);
    });


    //从分组删除设备
    serviceSiteManager->registerServiceRequestHandler(RemoveDeviceFromGroup_Device_Service_ID,
                                                      [&lc](const Request& request, Response& response) -> int{
        return removeDeviceFromGroup_service_handler(request, response, lc);
    });

    //控制分组
    serviceSiteManager->registerServiceRequestHandler(ControlGroup_Device_Service_ID,
                                                      [&lc](const Request& request, Response& response) -> int{
        return control_group_service_handler(request, response, lc);
    });

    //获取分组列表
    serviceSiteManager->registerServiceRequestHandler(GetGroupList_Device_Service_ID, getGroupList_service_handler);


    //注册白名单改变助理函数
    serviceSiteManager->registerMessageHandler(WhiteList_Changed, updateDeviceList);

    //订阅白名单改变消息
//    threadPool_.enqueue([&](){
//        while(true){
//            int code;
//            std::vector<string> messageIdList;
//            messageIdList.push_back(WhiteList_Changed);
//            code = serviceSiteManager->subscribeMessage(LocalIp, ConfigPort, messageIdList);
//
//            if (code == ServiceSiteManager::RET_CODE_OK) {
//                printf("subscribeMessage whiteListModifiedByAppMsg ok.\n");
//                break;
//            }
//
//            std::this_thread::sleep_for(std::chrono::seconds(3));
//            printf("subscribed whiteListModifiedByAppMsg failed....., start to subscribe in 3 seconds\n");
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
                LOG_RED << "===>bleSite startByRegister error, code = " << code;
                LOG_RED << "===>bleSite startByRegister in 3 seconds....";
                std::this_thread::sleep_for(std::chrono::seconds(3));
            }else{
                LOG_PURPLE << "===>bleSite startByRegister successfully.....";
                break;
            }
        }
    });

    while(true){
        std::this_thread::sleep_for(std::chrono::seconds(60 *10));
    }

    return 0;
}
