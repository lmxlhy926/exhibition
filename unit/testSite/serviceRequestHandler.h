//
// Created by 78472 on 2022/5/11.
//

#ifndef EXHIBITION_SERVICEREQUESTHANDLER_H
#define EXHIBITION_SERVICEREQUESTHANDLER_H

#include <string>
#include <socket/httplib.h>
#include "siteService/nlohmann/json.hpp"
#include "socket/socketServer.h"
#include "common/configParamUtil.h"

using namespace std;
using namespace httplib;
using json = nlohmann::json;


//获取白名单列表
int whiteList_service_get_handler(const Request& request, Response& response);

int whiteList_service_put_handler(const Request& request, Response& response);



#endif //EXHIBITION_SERVICEREQUESTHANDLER_H
