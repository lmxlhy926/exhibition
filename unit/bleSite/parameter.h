//
// Created by 78472 on 2022/5/18.
//

#ifndef EXHIBITION_PARAMETER_H
#define EXHIBITION_PARAMETER_H

#include <string>

using namespace std;

static const string LocalIp = "127.0.0.1";
static const string ConfigSiteName = "config";
static const int ConfigPort = 9006;

//站点描述
static const string BLE_SITE_ID = "ble_light";
static const string BLE_SITE_ID_NAME = "BLE灯控";
static const int BLE_SITE_PORT = 9001;

//服务ID
static const string Reset_Device_Service_ID               = "reset_device";                      //重置网关
static const string Scan_Device_Service_ID                = "scan_device";                       //扫描设备结果
static const string Add_Device_Service_ID                 = "add_device";                        //添加设备
static const string Del_Device_Service_ID                 = "del_device";                        //删除设备
static const string Control_Device_Service_ID             = "control_device";                    //控制设备
static const string Contorl_All_Device_Service_ID         = "controlAll_device";                 //控制所有设备
static const string Config_Device_Property_Service_ID     = "config_name_location";              //配置设备属性
static const string Get_DeviceList_Service_ID             = "get_device_list";                   //获取设备列表
static const string Get_DeviceList_byRoomName_Service_ID  = "get_device_list_byRoomName";        //依据房名名获取设备列表
static const string Save_DeviceList_Service_ID            = "save_device_list";                  //存储设备列表
static const string Get_DeviceState_Service_ID            = "get_device_state";                  //获取设备状态

//组服务ID
static const string CreateGroup_Device_Service_ID               = "create_group";                   //创建分组
static const string DeleteGroup_Device_Service_ID               = "delete_group";                   //删除分组
static const string RenameGroup_Device_Service_ID               = "rename_group";                   //重命名分组
static const string AddDevice2Group_Device_Service_ID           = "add_device_to_group";            //添加设备进分组
static const string AddDevice2GroupByRoom_Device_Service_ID     = "add_device_to_group_byRoom";     //按照指定的房间进行分组
static const string RemoveDeviceFromGroup_Device_Service_ID     = "remove_device_from_group";       //从分组删除设备
static const string ControlGroup_Device_Service_ID              = "control_group";                  //控制分组
static const string GetGroupList_Device_Service_ID              = "get_group_list";                 //获取分组列表

//二进制指令调试
static const string Ble_Device_Test_Command_Service_ID = "BleDeviceCommand";

//站点支持的消息
static const string ScanResultMsg =                    "scanResultMsg";                     //扫描结果消息
static const string SingleDeviceBindSuccessMsg =       "singleDeviceBindSuccessMsg";        //单个设备绑定成功
static const string SingleDeviceUnbindSuccessMsg =     "singleDeviceUnbindSuccessMsg";      //单个设备解绑成功
static const string BindEndMsg =                       "bindEndMsg";                        //绑定结束
static const string Device_State_Changed =             "device_state_changed";              //设备状态改变


//站点订阅的消息
static const string WhiteList_Changed =   "whiteListModifiedByAppMsg";      //app修改了白名单


//扫描绑定指令
#define SCAN                        "scan"
#define SCANEND                     "scanEnd"
#define BIND                        "bind"
#define UNBIND                      "unbind"
#define CONNECT                     "connect"
#define NETVERIFY                   "netVerify"
#define ASSIGN_GATEWAY_ADDRESS      "assignGateWayAddress"
#define ASSIGN_NODE_ADDRESS         "assignNodeAddress"
#define ADD_DEVICE                  "add_device"

//组控指令
#define AddDevice2Group             "addDevice2Group"
#define DelDeviceFromGroup          "delDeviceFromGroup"

//设备控制指令
#define POWER                       "power"
#define LUMINANCE                   "luminance"
#define COLORTEMPERATURE            "color_temperature"
#define LUMINANCECOLORTEMPERATURE   "luminance_color_temperature"
#define LUMINANCERELATIVE           "luminance_relative"
#define COLORTEMPERATURERELATIVE    "color_temperature_relative"
#define MODECONFIG                  "mode_config"

#endif //EXHIBITION_PARAMETER_H
