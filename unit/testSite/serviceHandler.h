//
// Created by 78472 on 2022/5/17.
//

#ifndef EXHIBITION_SERVICEHANDLER_H
#define EXHIBITION_SERVICEHANDLER_H

#include <string>
#include <socket/httplib.h>
#include "siteService/nlohmann/json.hpp"

using namespace std;
using namespace httplib;
using json = nlohmann::json;


static const string DEVICE_LIST_REQUEST_SERVICE_ID = "get_device_list";
static const string CONTROL_DEVICE_REGISTER_SERVICE_ID = "control_device";


int deviceList_service_request_handler(const Request& request, Response& response);

int controlDevice_service_request_handler(const Request& request, Response& response);




#endif //EXHIBITION_SERVICEHANDLER_H
