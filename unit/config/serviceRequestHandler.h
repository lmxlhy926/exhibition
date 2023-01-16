//
// Created by 78472 on 2022/5/11.
//

#ifndef EXHIBITION_SERVICEREQUESTHANDLER_H
#define EXHIBITION_SERVICEREQUESTHANDLER_H

#include <string>
#include <http/httplib.h>
#include "siteService/nlohmann/json.hpp"
#include "util/cloudUtil.h"
#include "util/configParamUtil.h"

using namespace std;
using namespace httplib;
using json = nlohmann::json;


//获取情景列表<内部>
int sceneListRequest_service_request_handler(const Request& request, Response& response, bool isConnec);

//子设备注册<内部>
int subDeviceRegister_service_request_handler(const Request& request, Response& response, bool isConnec);

//同步设备列表
int postDeviceList_service_request_handler(const Request& request, Response& response, bool isConnec);

//获取家庭域<内部>
int domainIdRequest_service_request_handler(const Request& request, Response& response, bool isConnec);

//安装app发送请求，传递安装师傅信息<安装app>
int engineer_service_request_handler(mqttClient& mc, const Request& request, Response& response);

//从云端主动获取白名单
int getWhiteListFromCloud_service_request_handler(mqttClient& mc, const Request& request, Response& response);


//获取白名单列表
int whiteList_service_request_handler(const Request& request, Response& response);

//app保存白名单列表
int whiteList_save_service_request_handler(const Request& request, Response& response);

//蓝牙站点添加新设备时，增加相应的条目
int whiteList_update_service_request_handler(const Request& request, Response& response);

//蓝牙站点解绑设备时，删除相应的条目
int whiteList_delete_service_request_handler(const Request& request, Response& response);

//获取设备列表，返回给智慧安装app
int getAllDeviceList_service_request_handler(const Request& request, Response& response);

//电视发声
int tvSound_service_request_handler(const Request& request, Response& response);

//获取场景配置文件
int getConfigFile_service_request_handler(const Request& request, Response& response);

//保存场景配置文件
int saveConfigFile_service_request_handler(const Request& request, Response& response);



#endif //EXHIBITION_SERVICEREQUESTHANDLER_H
