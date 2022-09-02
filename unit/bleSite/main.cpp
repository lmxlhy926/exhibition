
#include <thread>
#include <unistd.h>
#include <atomic>
#include <cstdio>
#include <functional>
#include <sstream>

#include "siteService/nlohmann/json.hpp"
#include "siteService/service_site_manager.h"

#include "qlibc/FileUtils.h"
#include "serviceRequestHandler.h"
#include "parameter.h"
#include "formatTrans/statusEvent.h"
#include "formatTrans/bleConfig.h"
#include "formatTrans/upBinayCmd.h"
#include "formatTrans/downBinaryCmd.h"
#include "formatTrans/downBinaryFlowControl.h"
#include "logic/logicControl.h"

using namespace std;
using namespace servicesite;
using namespace httplib;
using namespace std::placeholders;
using json = nlohmann::json;

static const string SYNERGY_SITE_ID = "BLE";
static const string SYNERGY_SITE_ID_NAME = "BLE";

int main(int argc, char* argv[]) {

    httplib::ThreadPool threadPool_(10);
    std::atomic<bool> http_server_thread_end(false);

    //流控
    downBinaryFlowControl::getInstance();

    // 创建 serviceSiteManager 对象, 单例
    ServiceSiteManager* serviceSiteManager = ServiceSiteManager::getInstance();
    serviceSiteManager->setServerPort(BleSitePort);

    //设置配置文件加载路径, 加载配置文件
    bleConfig* configPathPtr = bleConfig::getInstance();
    configPathPtr->setConfigPath(string(argv[1]));

    //注册串口上报回调函数，初始化并打开串口
    while(!configPathPtr->serialInit(UpBinaryCmd::bleReceiveFunc)){
        LOG_RED << "==>failed to open the serial<"
                  << configPathPtr->getSerialData().getString("serial") << ">....";
        LOG_RED << "===>try to open in 3 seconds.....";
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
    LOG_INFO << "===>success in open serial<"
              << configPathPtr->getSerialData().getString("serial") << ">";

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

    //获取设备列表
    serviceSiteManager->registerServiceRequestHandler(Get_DeviceList_Service_ID,
                                                      [](const Request& request, Response& response) -> int{
        return get_device_list_service_handler(request, response);
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


    // 站点监听线程启动
    threadPool_.enqueue([&](){
        while(true){
            //自启动方式
            int code = serviceSiteManager->start();
            //注册启动方式
//            int code = serviceSiteManager->startByRegister();
            if(code != 0){
                std::cout << "===>bleSite startByRegister error, code = " << code << std::endl;
                std::cout << "===>bleSite startByRegister in 3 seconds...." << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(3));
            }else{
                std::cout << "===>bleSite startByRegister successfully....." << std::endl;
                break;
            }
        }
    });

    while(true){
        std::this_thread::sleep_for(std::chrono::seconds(60 *10));
    }

    return 0;
}
