//
// Created by 78472 on 2022/5/18.
//

#ifndef EXHIBITION_PARAM_H
#define EXHIBITION_PARAM_H

static const string RequestIp = "127.0.0.1";
static const int ZigBeeSitePort = 9002;
static const int ConfigSitePort = 9006;
static const int AdapterPort = 9008;
static const int SouthPort = 9010;
static const int QuerySitePort = 9000;

//站点描述
static const string CONFIG_SITE_ID = "config";
static const string CONFIG_SITE_ID_NAME = "整体配置";

//请求url
static const string SCENELIST_URL           =  "/logic-device/scene/list";                   //请求场景列表URL
static const string SUBDEVICE_REGISTER_URL  =  "/logic-device/edge/deviceRegister";          //子设备注册
static const string POSTDEVICELIST_URL      =  "/logic-device/edge/postDevicesList";         //设备同步
static const string WHITELIST_REQUEST_URL   =  "/logic-device/edge/getLittleWhiteList";      //获取白名单

//服务ID, 对内提供
static const string SCENELIST_REQUEST_SERVICE_ID            =    "sceneListRequest";
static const string SUBDEVICE_REGISTER_SERVICE_ID           =    "subDeviceRegister";
static const string POSTDEVICELIST_REGISTER_SERVICE_ID      =    "postDeviceList";
static const string DOMAINID_REQUEST_SERVICE_ID             =    "domainIdRequest";
static const string ENGINEER_REQUEST_SERVICE_ID             =    "engineerAppInfo";
static const string WHITELISTCLOUD_REQUEST_SERVICE_ID       =    "whiteListRequestCloud";

static const string WHITELIST_REQUEST_SERVICE_ID                    =    "whiteListRequest";
static const string WHITELIST_SAVE_REQUEST_SERVICE_ID               =    "whiteListSaveRequest";
static const string WHITELIST_SYNC_SAVE_REQUEST_SERVICE_ID          =    "whiteListSyncSaveRequest";
static const string GET_SCENECONFIG_FILE_REQUEST_SERVICE_ID         =    "getSceneConfigFile";
static const string SAVE_SCENECONFIG_FILE_REQUEST_SERVICE_ID        =    "saveSceneConfigFile";
static const string SAVE_SYNC_SCENECONFIGFILE_REQUEST_SERVICE_ID    =    "saveSceneConfigFileSync";
static const string GETALLLIST_REQUEST_SERVICE_ID                   =    "get_all_device_list";
static const string SETPANELINFO_REQUEST_SERVICE_ID                 =    "config_self_info";
static const string GETPANELINFO_REQUEST_SERVICE_ID                 =    "get_self_info";
static const string SETAUDIOPANELLIST_REQUEST_SERVICE_ID            =    "setAudioPanelList";
static const string GETAUDIOPANELLIST_REQUEST_SERVICE_ID            =    "getAudioPanelList";
static const string SETRADARLIST_REQUEST_SERVICE_ID                 =    "setRadarPanelList";



//消息发布ID
static const string WHITELIST_MESSAGE_ID                =  "whiteList";             //通知白名单被修改
static const string RECEIVED_WHITELIST_ID               = "receivedWhiteList";      //通知安装师傅app，收到了白名单
static const string WHITELIST_MODIFIED_MESSAGE_ID       = "whiteList";              //白名单被app修改
static const string SCENELIST_MODIFIED_MESSAGE_ID       = "sceneListModified";      //场景文件被app修改
static const string PANELINFO_MODIFIED_MESSAGE_ID       = "self_info_update";       //面板配置信息更改
static const string RADARDEVICE_RECEIVED_MESSAGE_ID     = "radarDeviceRegister";    //接收雷达设备


#endif //EXHIBITION_PARAM_H
