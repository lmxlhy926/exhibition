//
// Created by 78472 on 2022/5/11.
//

#ifndef EXHIBITION_SUBSCRIBEMESSAGEHANDLER_H
#define EXHIBITION_SUBSCRIBEMESSAGEHANDLER_H

#include <string>
#include "socket/httplib.h"
#include "socket/socketServer.h"
#include "siteService/service_site_manager.h"


using namespace std;
using namespace httplib;
using namespace servicesite;

//消息ID
static const string DEVICESTATUS_MESSAGE_ID = "xxx";
static const string RADAR_MESSAGE_ID = "xxxx";
static const string MICPANELMESSAGE_ID = "xxxxx";


void deviceStatus_message_handler(socketServer& server, const Request& request);

void radar_message_handler(socketServer& server, const Request& request);

void micpanel_message_handler(socketServer& server, const Request& request);





#endif //EXHIBITION_SUBSCRIBEMESSAGEHANDLER_H
