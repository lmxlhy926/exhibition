//
// Created by 78472 on 2022/5/18.
//

#ifndef EXHIBITION_PARAMCONFIG_H
#define EXHIBITION_PARAMCONFIG_H


static const string AdapterIp = "127.0.0.1";
static const int AdapterPort = 60004;

static const int ConfigSitePort = 9006;

//请求url
static const string SCENELIST_URL = "/logic-device/scene/list";  //请求场景列表URL
static const string SUBDEVICE_REGISTER_URL = "/logic-device/edge/deviceRegister";
static const string WHITELIST_REQUEST_URL =  "/logic-device/edge/deviceRegister";

//服务ID, 对内提供
static const string SCENELIST_REQUEST_SERVICE_ID = "sceneListRequest";
static const string SUBDEVICE_REGISTER_SERVICE_ID = "subDeviceRegister";
static const string DOMAINID_REQUEST_SERVICE_ID = "domainIdRequest";
static const string ENGINEER_REQUEST_SERVICE_ID = "engineerAppInfo";
static const string WHITELIST_REQUEST_SERVICE_ID = "whiteListRequest";















#endif //EXHIBITION_PARAMCONFIG_H
