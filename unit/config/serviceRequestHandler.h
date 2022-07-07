//
// Created by 78472 on 2022/5/11.
//

#ifndef EXHIBITION_SERVICEREQUESTHANDLER_H
#define EXHIBITION_SERVICEREQUESTHANDLER_H

#include <string>
#include <common/httplib.h>
#include "siteService/nlohmann/json.hpp"
#include "socket/socketServer.h"
#include "util/cloudUtil.h"
#include "util/configParamUtil.h"

using namespace std;
using namespace httplib;
using json = nlohmann::json;


//获取情景列表<内部>
int sceneListRequest_service_request_handler(const Request& request, Response& response);

//子设备注册<内部>
int subDeviceRegister_service_request_handler(const Request& request, Response& response);

//获取家庭域<内部>
int domainIdRequest_service_request_handler(const Request& request, Response& response);

//安装app发送请求，传递安装师傅信息<安装app>
int engineer_service_request_handler(mqttClient& mc, const Request& request, Response& response);

//获取白名单列表
int whiteList_service_request_handler(const Request& request, Response& response);

//获取灯控设备设备列表
int getAllDeviceList_service_request_handler(const Request& request, Response& response);

//电视发声
int tvSound_service_request_handler(const Request& request, Response& response);




#endif //EXHIBITION_SERVICEREQUESTHANDLER_H
