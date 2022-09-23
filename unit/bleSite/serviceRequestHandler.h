//
// Created by 78472 on 2022/5/15.
//

#ifndef EXHIBITION_SERVICEREQUESTHANDLER_H
#define EXHIBITION_SERVICEREQUESTHANDLER_H

#include <string>
#include "common/httplib.h"
#include "logic/logicControl.h"
using namespace httplib;

//扫描设备
int scan_device_service_handler(const Request& request, Response& response, LogicControl& lc);

//添加设备
int add_device_service_handler(const Request& request, Response& response, LogicControl& lc);

//删除设备
int del_device_service_handler(const Request& request, Response& response, LogicControl& lc);

//控制设备
int control_device_service_handler(const Request& request, Response& response, LogicControl& lc);

//控制所有设备
int control_all_service_handler(const Request& request, Response& response, LogicControl& lc);

//获取设备列表
int get_device_list_service_handler(const Request& request, Response& response);

//获取设备状态
int get_device_state_service_handler(const Request& request, Response& response);

//存储设备列表
int save_deviceList_service_handler(const Request& request, Response& response);

//二进制控制指令
int BleDevice_command_test_service_handler(const Request& request, Response& response);


//创建分组
int create_group_service_handler(const Request& request, Response& response);

//删除分组
int delete_group_service_handler(const Request& request, Response& response, LogicControl& lc);

//重命名分组
int rename_group_service_handler(const Request& request, Response& response);

//添加设备进入分组
int addDevice2Group_service_handler(const Request& request, Response& response, LogicControl& lc);

//从分组移除设备
int removeDeviceFromGroup_service_handler(const Request& request, Response& response, LogicControl& lc);

//控制分组
int control_group_service_handler(const Request& request, Response& response, LogicControl& lc);

//获取分组列表
int getGroupList_service_handler(const Request& request, Response& response);


//通过白名单列表更新设备列表信息
void updateDeviceList(const Request& request);


#endif //EXHIBITION_SERVICEREQUESTHANDLER_H
