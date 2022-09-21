//
// Created by 78472 on 2022/5/15.
//

#ifndef EXHIBITION_SERVICEREQUESTHANDLER_H
#define EXHIBITION_SERVICEREQUESTHANDLER_H

#include <string>
#include <common/httplib.h>
using namespace httplib;

int device_control_service_handler(const Request& request, Response& response);


int getDeviceList_service_handler(const Request& request, Response& response);



#endif //EXHIBITION_SERVICEREQUESTHANDLER_H
