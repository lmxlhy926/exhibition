//
// Created by 78472 on 2022/11/17.
//

#ifndef EXHIBITION_SERVICEREQUESTHANDLER_H
#define EXHIBITION_SERVICEREQUESTHANDLER_H

#include <string>
#include <http/httplib.h>
using namespace httplib;

//开启mdns服务
void mdnsServiceStart();

//主动查询服务
int site_discovery_service_handler(const Request& request, Response& response);


#endif //EXHIBITION_SERVICEREQUESTHANDLER_H
