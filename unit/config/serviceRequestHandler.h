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

//获取家庭域ID
int domainIdRequest_service_request_handler(const Request& request, Response& response, bool isConnec);

//接收app发送的安装师傅信息，注册电视，保存安装信息，置位注册电视成功标志
int engineer_service_request_handler(mqttClient& mc, const Request& request, Response& response);

//从云端主动获取白名单，转换并保存白名单，但是不发布消息
int getWhiteListFromCloud_service_request_handler(mqttClient& mc, const Request& request, Response& response);


//-----------------------------------------------------------------------------------------------------------------
//同步白名单
void whiteList_sync(string site_id, string getServiceId, string saveServiceId);

//白名单同步保存，不对外开放
int whiteList_sync_save_service_request_handler(const Request& request, Response& response);

//获取白名单列表
int whiteList_service_request_handler(const Request& request, Response& response);

//保存白名单，并将白名单同步到每个配置站点
int whiteList_save_service_request_handler(const Request& request, Response& response);

//蓝牙设备列表对白名单进行更新, 不对外开放
int whiteList_update_service_request_handler(const Request& request, Response& response);

//获取场景配置文件
int getSceneFile_service_request_handler(const Request& request, Response& response);

//保存场景配置文件
int saveSceneFile_service_request_handler(const Request& request, Response& response);

//场景文件同步保存
int saveSceneFile_sync_service_request_handler(const Request& request, Response& response);

//获取设备列表，返回给智慧安装app
int getAllDeviceList_service_request_handler(const Request& request, Response& response);

//设置面板信息
int setPanelInfo_service_request_handler(const Request& request, Response& response);

//获取面板信息
int getPanelInfo_service_request_handler(const Request& request, Response& response);


#endif //EXHIBITION_SERVICEREQUESTHANDLER_H
