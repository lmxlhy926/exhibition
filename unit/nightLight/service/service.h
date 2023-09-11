#ifndef NIGHTLIGHT_SERVICE_H
#define NIGHTLIGHT_SERVICE_H

#include "siteService/service_site_manager.h"


//雷达点位处理
void radarMessageHandle(const Request& request);

//接收雷达点位
int radarPoint_service_request_handler(const Request& request, Response& response);

//保存夜灯灯带
int saveStrip_service_request_handler(const Request& request, Response& response);


//删除夜灯灯带
int delStrip_service_request_handler(const Request& request, Response& response);


//获取灯带列表
int getStripList_service_request_handler(const Request& request, Response& response);

#endif