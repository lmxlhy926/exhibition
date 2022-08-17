//
// Created by 78472 on 2022/5/15.
//

#ifndef EXHIBITION_SERVICEREQUESTHANDLER_H
#define EXHIBITION_SERVICEREQUESTHANDLER_H

#include <string>
#include "common/httplib.h"
#include "logic/logicControl.h"
using namespace httplib;

//json控制指令
int BleDevice_command_service_handler(const Request& request, Response& response, LogicControl& lc);

//二进制控制指令
int BleDevice_command_test_service_handler(const Request& request, Response& response);

//添加设备
int add_device_service_handler(const Request& request, Response& response, LogicControl& lc);

//删除设备
int del_device_service_handler(const Request& request, Response& response, LogicControl& lc);

//控制设备
int control_device_service_handler(const Request& request, Response& response, LogicControl& lc);

//获取设备列表
int get_device_list_service_handler(const Request& request, Response& response, LogicControl& lc);

//获取设备状态
int get_device_state_service_handler(const Request& request, Response& response, LogicControl& lc);









#endif //EXHIBITION_SERVICEREQUESTHANDLER_H
