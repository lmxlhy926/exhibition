#ifndef NIGHTLIGHT_PARAM_H
#define NIGHTLIGHT_PARAM_H

#include <string>
using namespace std;

static const string RequestIp = "127.0.0.1";

static const string QuerySiteID = "site_query";
static const string QuerySiteName = "服务站点查询";
static const int QuerySitePort = 9000;

static const string BleSiteID = "ble_light";
static const string BleSiteName = "BLE灯控";
static const int BleSitePort = 9001;

static const string ZigbeeSiteID = "zigbee_light";
static const string ZigbeeSiteName = "ZigBee灯控";
static const int ZigbeeSitePort = 9002;

static const string SynergySiteID = "collaborate";
static const string SynergySiteName = "协同服务";
static const int SynergySitePort = 9007;


//服务ID
static const string Control_Service_ID              = "cloudCommand";
static const string VoiceControl_Service_ID         = "voiceControl";
static const string BleGroupRegister_Service_ID     = "register_bleGroup";
static const string BleDeviceOperation_Service_ID   = "bleDeviceOperation";
static const string UpdateDeviceList_Service_ID     = "updateDeviceList";
static const string UpdateGroupList_Service_ID      = "updateGroupList";
static const string GetPanelList_Service_ID         = "get_panelList";                  //获取面板列表

//服务ID
static const string Reset_Device_Service_ID               = "reset_device";                      //重置网关
static const string Scan_Device_Service_ID                = "scan_device";                       //扫描设备结果
static const string Add_Device_Service_ID                 = "add_device";                        //添加设备
static const string AddZigbee_Device_Service_ID           = "allow_device_join";                 //添加zigbee设备
static const string Del_Device_Service_ID                 = "del_device";                        //删除设备
static const string ConfigProperty_Service_ID             = "config_name_location";              //修改设备属性信息
static const string Control_Device_Service_ID             = "control_device";                    //控制设备
static const string Get_DeviceList_Service_ID             = "get_device_list";                   //获取设备列表
static const string Get_DeviceState_Service_ID            = "get_device_state";                  //获取设备状态


//订阅消息ID
static const string Site_OnOffLine_MessageID        =   "site_onoffline";
//订阅蓝牙站点的消息
static const string ScanResultMsg                   =   "scanResultMsg";                     //扫描结果消息
static const string SingleDeviceBindSuccessMsg      =   "singleDeviceBindSuccessMsg";        //单个设备绑定成功
static const string SingleDeviceUnbindSuccessMsg    =   "singleDeviceUnbindSuccessMsg";      //单个设备解绑成功
static const string BindEndMsg                      =   "bindEndMsg";                        //绑定结束
static const string Device_State_Changed            =   "device_state_changed";              //设备状态改变
//订阅的zigbee站点消息
static const string DeviceOnOffLineMsg              =   "device_on_off_line";                //设备上下线
static const string TriggerSceneMsg                 =   "trigger_scene";                     //场景触发


//发布消息ID
static const string Scene_Msg_MessageID             =   "sceneMsg";
static const string DeviceGroupList_Update_Message  =   "deviceGroupListUpdateMsg";


#endif