//
// Created by 78472 on 2022/5/11.
//

#ifndef EXHIBITION_SERVICEREQUESTHANDLER_H
#define EXHIBITION_SERVICEREQUESTHANDLER_H

#include <string>
#include <socket/httplib.h>
#include "siteService/nlohmann/json.hpp"
#include "socket/socketServer.h"
#include "cloudUtil.h"
#include "common/configParamUtil.h"

using namespace std;
using namespace httplib;
using json = nlohmann::json;


static const string SCENELIST_URL = "/logic-device/scene/list";  //请求场景列表URL
static const string SUBDEVICE_REGISTER_URL = "/logic-device/edge/deviceRegister";

//服务ID, 对内提供
static const string SCENELIST_REQUEST_SERVICE_ID = "sceneListRequest";
static const string SUBDEVICE_REGISTER_SERVICE_ID = "subDeviceRegister";
static const string DOMAINID_REQUEST_SERVICE_ID = "domainIdRequest";
static const string ENGINEER_REQUEST_SERVICE_ID = "engineerAppInfo";


//获取情景列表<内部>
int sceneListRequest_service_request_handler(const Request& request, Response& response);

//子设备注册<内部>
int subDeviceRegister_service_request_handler(const Request& request, Response& response);

//获取家庭域<内部>
int domainIdRequest_service_request_handler(const Request& request, Response& response);

//安装app发送请求，传递安装师傅信息<安装app>
int engineer_service_request_handler(mqttClient& mc, const Request& request, Response& response);


#endif //EXHIBITION_SERVICEREQUESTHANDLER_H
