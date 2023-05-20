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
/*
 *  1. 从主从站点获取所有面板列表
 *  2. 得到面板ip后，访问每个面板上的配置站点，获取其白名单
 *  3. 白名单按照从旧到新，叠加覆盖删除
 */
void whiteListFileSync(string site_id, string getServiceId, string message);

//场景文件同步：比较时间戳，最新的覆盖旧的
void sceneFileSync(string site_id, string getServiceId, string message);

//获取白名单列表
int whiteList_get_service_request_handler(const Request& request, Response& response);

//保存白名单，并将白名单同步到每个配置站点
int whiteList_save_service_request_handler(const Request& request, Response& response);

//获取场景配置文件
int getSceneFile_service_request_handler(const Request& request, Response& response);

//保存场景配置文件
int saveSceneFile_service_request_handler(const Request& request, Response& response);

//获取设备列表，返回给智慧安装app
int getAllDeviceList_service_request_handler(const Request& request, Response& response);

//获取面板信息
int getPanelInfo_service_request_handler(const Request& request, Response& response);

//设置面板信息
int setPanelInfo_service_request_handler(const Request& request, Response& response);

//获取语音面板列表
int getAudioPanelList_service_request_handler(const Request& request, Response& response);

//app推送账户下的语音面板全集
int saveAudioPanelList_service_request_handler(const Request& request, Response& response);

//app推送账号下的所有雷达设备全集
int setRadarDevice_service_request_handler(const Request& request, Response& response);

#if 0
//订阅雷达消息
void radarMessageReceivedHandler(const Request& request);
#endif

#endif //EXHIBITION_SERVICEREQUESTHANDLER_H
