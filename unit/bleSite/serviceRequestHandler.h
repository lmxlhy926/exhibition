//
// Created by 78472 on 2022/5/15.
//

#ifndef EXHIBITION_SERVICEREQUESTHANDLER_H
#define EXHIBITION_SERVICEREQUESTHANDLER_H

#include <string>
#include "common/httplib.h"
#include "logic/logicControl.h"
using namespace httplib;


int BleDevice_command_service_handler(const Request& request, Response& response, LogicControl& lc);

int BleDevice_command_test_service_handler(const Request& request, Response& response);


#endif //EXHIBITION_SERVICEREQUESTHANDLER_H
