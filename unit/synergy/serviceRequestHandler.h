//
// Created by 78472 on 2022/5/15.
//

#ifndef EXHIBITION_SERVICEREQUESTHANDLER_H
#define EXHIBITION_SERVICEREQUESTHANDLER_H

#include <string>
#include <common/httplib.h>
using namespace httplib;

//设备控制
int device_control_service_handler(const Request& request, Response& response);

//获取设备列表：ble, zigbee, tvAdapter
int getDeviceList_service_handler(const Request& request, Response& response);

//场景控制指令
int sceneCommand_service_handler(const Request& request, Response& response);


#endif //EXHIBITION_SERVICEREQUESTHANDLER_H
