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

//设备消息订阅ID
static const string TVADAPTER_DEVICE_STATUS_MESSAGE_ID  = "";                           //tvAdapter设备消息订阅ID
static const string RADAR_DEVICE_STATUS_MESSAGE_ID      = "";                           //南向站点消息订阅ID
static const string REGISTERAGAIN_MESSAGE_ID            = "register2QuerySiteAgain";    //重新向南向站点注册
static const string FileSync_MESSAGE_ID                 = "fileSync";                   //文件同步


//消息发布ID
static const string TVSOUND_MESSAGE_ID = "getTvSound";                              //电视发声事件上报
static const string DEVICE_STATUS_MESSAGE_ID = "allDeviceStatus";                   //设备状态上报
static const string WHITELIST_MODIFIED_MESSAGE_ID = "whiteListModifiedByAppMsg";    //白名单被app修改

//请求url
static const string SCENELIST_URL           =  "/logic-device/scene/list";                   //请求场景列表URL
static const string SUBDEVICE_REGISTER_URL  =  "/logic-device/edge/deviceRegister";          //子设备注册
static const string POSTDEVICELIST_URL      =  "/logic-device/edge/postDevicesList";         //设备同步
static const string WHITELIST_REQUEST_URL   =  "/logic-device/edge/getLittleWhiteList";      //获取白名单

//服务ID, 对内提供
static const string SCENELIST_REQUEST_SERVICE_ID =          "sceneListRequest";
static const string SUBDEVICE_REGISTER_SERVICE_ID =         "subDeviceRegister";
static const string POSTDEVICELIST_REGISTER_SERVICE_ID =    "postDeviceList";
static const string DOMAINID_REQUEST_SERVICE_ID =           "domainIdRequest";
static const string ENGINEER_REQUEST_SERVICE_ID =           "engineerAppInfo";
static const string WHITELIST_REQUEST_SERVICE_ID =          "whiteListRequest";
static const string WHITELISTCLOUD_REQUEST_SERVICE_ID =     "whiteListRequestCloud";
static const string WHITELIST_UPDATE_REQUEST_SERVICE_ID =   "whiteListUpdateRequest";
static const string WHITELIST_DELETE_REQUEST_SERVICE_ID =   "whiteListDeleteRequest";
static const string WHITELIST_SAVE_REQUEST_SERVICE_ID =     "whiteListSaveRequest";
static const string GETALLLIST_REQUEST_SERVICE_ID =         "get_all_device_list";
static const string TVSOUND_REQUEST_SERVICE_ID =            "tvSound";

static const string GETSCENECONFIGFILE_REQUEST_SERVICE_ID =      "getSceneConfigFile";
static const string SAVESCENECONFIGFILE_REQUEST_SERVICE_ID =     "saveSceneConfigFile";



#endif //EXHIBITION_PARAM_H
