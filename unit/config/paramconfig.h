//
// Created by 78472 on 2022/5/18.
//

#ifndef EXHIBITION_PARAMCONFIG_H
#define EXHIBITION_PARAMCONFIG_H


static const string RequestIp = "192.168.50.138";
static const int AdapterPort = 9008;
static const int SouthPort = 9010;
static const int ConfigSitePort = 9006;
static const int ZigBeeSitePort = 9002;



//设备消息订阅ID
static const string TVADAPTER_DEVICE_STATUS_MESSAGE_ID = "";        //tvAdapter设备消息订阅ID
static const string RADAR_DEVICE_STATUS_MESSAGE_ID = "";            //南向站点消息订阅ID

//消息发布ID
static const string TVSOUND_MESSAGE_ID = "getTvSound";              //电视发声事件上报
static const string DEVICE_STATUS_MESSAGE_ID = "allDeviceStatus";   //设备状态上报

//请求url
static const string SCENELIST_URL = "/logic-device/scene/list";  //请求场景列表URL
static const string SUBDEVICE_REGISTER_URL = "/logic-device/edge/deviceRegister";
static const string WHITELIST_REQUEST_URL =  "";

//服务ID, 对内提供
static const string SCENELIST_REQUEST_SERVICE_ID = "sceneListRequest";
static const string SUBDEVICE_REGISTER_SERVICE_ID = "subDeviceRegister";
static const string DOMAINID_REQUEST_SERVICE_ID = "domainIdRequest";
static const string ENGINEER_REQUEST_SERVICE_ID = "engineerAppInfo";
static const string WHITELIST_REQUEST_SERVICE_ID = "whiteListRequest";
static const string GETALLLIST_REQUEST_SERVICE_ID = "get_all_device_list";
static const string TVSOUND_REQUEST_SERVICE_ID = "tvSound";


#endif //EXHIBITION_PARAMCONFIG_H
