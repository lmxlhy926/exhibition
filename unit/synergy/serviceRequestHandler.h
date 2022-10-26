//
// Created by 78472 on 2022/5/15.
//

#ifndef EXHIBITION_SERVICEREQUESTHANDLER_H
#define EXHIBITION_SERVICEREQUESTHANDLER_H

#include <string>
#include <http/httplib.h>
using namespace httplib;

namespace synergy{
//设备控制
    int cloudCommand_service_handler(const Request& request, Response& response);

//获取设备列表：ble, zigbee, tvAdapter
    int getDeviceList_service_handler(const Request& request, Response& response);

//获取分组列表
    int getGroupList_service_handler(const Request& request, Response& response);
}

#endif //EXHIBITION_SERVICEREQUESTHANDLER_H
