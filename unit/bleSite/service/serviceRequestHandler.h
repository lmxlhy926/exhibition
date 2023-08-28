//
// Created by 78472 on 2022/5/15.
//

#ifndef EXHIBITION_SERVICEREQUESTHANDLER_H
#define EXHIBITION_SERVICEREQUESTHANDLER_H

#include <string>
#include "http/httplib.h"
#include "downCmd/logicControl.h"
using namespace httplib;

//重置网关
int reset_device_service_handler(const Request& request, Response& response, LogicControl& lc);

//获取扫描结果
int scan_device_service_handler(const Request& request, Response& response, LogicControl& lc);

//发送扫描指令
int scanDevice_service_handler(const Request& request, Response& response, LogicControl& lc);

//添加设备
int add_device_service_handler(const Request& request, Response& response, LogicControl& lc);

//删除设备
int del_device_service_handler(const Request& request, Response& response, LogicControl& lc);

//强制删除设备
int del_device_force_service_handler(const Request& request, Response& response, LogicControl& lc);

//控制设备
int control_device_service_handler(const Request& request, Response& response, LogicControl& lc);

//获取设备列表
int get_device_list_service_handler(const Request& request, Response& response);

//获取指定房间的设备列表
int get_device_list_byRoomName_service_handler(const Request& request, Response& response);

//获取设备状态
int get_device_state_service_handler(const Request& request, Response& response);


//存储设备列表
int save_deviceList_service_handler(const Request& request, Response& response);

//二进制控制指令
int BleDevice_command_test_service_handler(const Request& request, Response& response);

//修改设备信息: 只能修改设备名称和位置
int device_config_service_handler(const Request& request, Response& response);


//创建分组
int create_group_service_handler(const Request& request, Response& response);

//删除分组，删除前先将设备移除分组
int delete_group_service_handler(const Request& request, Response& response, LogicControl& lc);

//重命名分组
int rename_group_service_handler(const Request& request, Response& response);

//添加设备进入分组
int addDevice2Group_service_handler(const Request& request, Response& response, LogicControl& lc);

//按房间进行分组
int groupByRoomname_service_handler(const Request& request, Response& response, LogicControl& lc);

//从分组移除设备
int removeDeviceFromGroup_service_handler(const Request& request, Response& response, LogicControl& lc);

//控制分组
int control_group_service_handler(const Request& request, Response& response, LogicControl& lc);

//获取分组列表
int getGroupList_service_handler(const Request& request, Response& response);

//测试接口
int test_service_handler(const Request& request, Response& response);

#endif //EXHIBITION_SERVICEREQUESTHANDLER_H
