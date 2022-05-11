//
// Created by 78472 on 2022/5/11.
//

#ifndef EXHIBITION_SERVICEREQUESTHANDLER_H
#define EXHIBITION_SERVICEREQUESTHANDLER_H

#include <string>
#include <socket/httplib.h>
#include "siteService/nlohmann/json.hpp"
#include "socket/socketServer.h"


using namespace std;
using namespace httplib;
using json = nlohmann::json;

//服务ID, 对内提供
static const string SCENELIST_REQUEST_SERVICE_ID = "sceneListRequest";
static const string SUBDEVICE_REGISTER_SERVICE_ID = "subDeviceRegister";
static const string DOMAINID_REQUEST_SERVICE_ID = "domainIdRequest";
//服务ID，对外提供
static const string ENGINEER_REQUEST_SERVICE_ID = "postEngineerInfo";
static const string GETDEVICELIST_REQUEST_SERVICE_ID = "getDeviceList";
static const string GETTVINFO_REQUEST_SERVICE_ID = "getTvInfo";
static const string CONTROLDEVICE_REQUEST_SERVICE_ID = "controlDevice";
static const string REDARREPORT_REQUEST_SERVICE_ID = "radar";

int sceneListRequest_service_request_handler(const Request& request, Response& response);

int subDeviceRegister_service_request_handler(const Request& request, Response& response);

int domainIdRequest_service_request_handler(const Request& request, Response& response);

int engineer_service_request_handler(const Request& request, Response& response);

int getDeviceList_service_request_handler(const Request& request, Response& response);

int getTvInfo_service_request_handler(const Request& request, Response& response);

int controlDevice_service_request_handler(const Request& request, Response& response);

int radarReportEnable_service_request_handler(const Request& request, Response& response);


#endif //EXHIBITION_SERVICEREQUESTHANDLER_H
