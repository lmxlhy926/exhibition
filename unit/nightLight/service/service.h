#ifndef NIGHTLIGHT_SERVICE_H
#define NIGHTLIGHT_SERVICE_H

#include "siteService/service_site_manager.h"


//蓝牙设备状态消息发布
void radarMessageHandle(const Request& request);

//保存夜灯灯带
int saveStrip_service_request_handler(const Request& request, Response& response);


//删除夜灯灯带
int delStrip_service_request_handler(const Request& request, Response& response);


//获取灯带列表
int getStripList_service_request_handler(const Request& request, Response& response);

#endif