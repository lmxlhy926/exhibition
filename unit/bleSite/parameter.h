//
// Created by 78472 on 2022/5/18.
//

#ifndef EXHIBITION_PARAMETER_H
#define EXHIBITION_PARAMETER_H

#include <string>

using namespace std;

const int BleSitePort = 60009;

//服务ID
static const string Control_Device_Service_ID     = "control_device";       //控制设备
static const string Add_Device_Service_ID         = "add_device";           //添加设备
static const string Del_Device_Service_ID         = "del_device";           //删除设备
static const string Get_DeviceList_Service_ID     = "get_device_list";      //获取设备列表
static const string Get_DeviceState_Service_ID    = "get_device_state";     //获取设备状态
static const string Scan_Device_Service_ID        = "scan_device";          //扫描设备结果

//暂时服务ID
static const string Ble_Device_Command_Service_ID = "BleDeviceCommand";
static const string Ble_Device_Test_Command_Service_ID = "BleDeviceCommands";


//站点支持的消息
static const string ScanResultMsg =                    "scanResult";
static const string SingleDeviceBindSuccessMsg =       "singleDeviceBindSuccess";
static const string SingleDeviceUnbindSuccessMsg =     "singleDeviceUnbindSuccess";
static const string BindEndMsg =                       "bindEnd";
static const string Device_State_Changed =          "device_state_changed";



//控制命令
#define SCAN                        "scan"
#define SCANEND                     "scanEnd"
#define BIND                        "bind"
#define UNBIND                      "unbind"
#define CONNECT                     "connect"
#define ASSIGN_GATEWAY_ADDRESS      "assignGateWayAddress"
#define ASSIGN_NODE_ADDRESS         "assignNodeAddress"
#define POWER                       "power"
#define LUMINANCE                   "luminance"
#define COLORTEMPERATURE            "color_temperature"



#endif //EXHIBITION_PARAMETER_H
