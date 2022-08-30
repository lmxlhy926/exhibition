//
// Created by 78472 on 2022/5/18.
//

#ifndef EXHIBITION_PARAMETER_H
#define EXHIBITION_PARAMETER_H

#include <string>

using namespace std;

static const string Ip = "127.0.0.1";
static const int ConfigPort = 9006;
static const int BleSitePort = 60009;

//服务ID
static const string Control_Device_Service_ID     = "control_device";       //控制设备
static const string Add_Device_Service_ID         = "add_device";           //添加设备
static const string Del_Device_Service_ID         = "del_device";           //删除设备
static const string Group_Device_Service_ID       = "group_device";         //设备分组
static const string Get_DeviceList_Service_ID     = "get_device_list";      //获取设备列表
static const string Get_DeviceState_Service_ID    = "get_device_state";     //获取设备状态
static const string Scan_Device_Service_ID        = "scan_device";          //扫描设备结果

//二进制指令调试
static const string Ble_Device_Test_Command_Service_ID = "BleDeviceCommand";

//站点支持的消息
static const string ScanResultMsg =                    "scanResultMsg";
static const string SingleDeviceBindSuccessMsg =       "singleDeviceBindSuccessMsg";
static const string SingleDeviceUnbindSuccessMsg =     "singleDeviceUnbindSuccessMsg";
static const string BindEndMsg =                       "bindEndMsg";
static const string Device_State_Changed =             "device_state_changed";


//控制命令
#define SCAN                        "scan"
#define SCANEND                     "scanEnd"
#define BIND                        "bind"
#define UNBIND                      "unbind"
#define CONNECT                     "connect"
#define ASSIGN_GATEWAY_ADDRESS      "assignGateWayAddress"
#define ASSIGN_NODE_ADDRESS         "assignNodeAddress"
#define GROUP                       "group"

#define POWER                       "power"
#define LUMINANCE                   "luminance"
#define COLORTEMPERATURE            "color_temperature"



#endif //EXHIBITION_PARAMETER_H
