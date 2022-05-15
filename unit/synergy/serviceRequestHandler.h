//
// Created by 78472 on 2022/5/15.
//

#ifndef EXHIBITION_SERVICEREQUESTHANDLER_H
#define EXHIBITION_SERVICEREQUESTHANDLER_H

#include <string>
#include <socket/httplib.h>
#include "socket/socketClient.h"



using namespace std;
using namespace httplib;

//服务ID, 对内提供
static const string TVUPLOAD_SERVICE_ID = "tveventUpload";
static const string SENSOR_SERVICE_ID = "sensorEventUpload";


int tvupload_service_handler(socketClient& client, const Request& request, Response& response);

int sensor_service_handler(socketClient& client, const Request& request, Response& response);




#endif //EXHIBITION_SERVICEREQUESTHANDLER_H
