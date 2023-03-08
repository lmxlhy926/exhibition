//
// Created by 78472 on 2022/5/15.
//

#ifndef EXHIBITION_SERVICEREQUESTHANDLER_H
#define EXHIBITION_SERVICEREQUESTHANDLER_H

#include <string>
#include <http/httplib.h>
using namespace httplib;

namespace synergy{
//判断站点是否在线（通过请求站点）
    bool isSiteOnline(const std::string& siteId);

//场景指令转换解析 + 设备控制（软服请求）
    int cloudCommand_service_handler(const Request& request, Response& response);

//语音控制入口
    int voiceControl_service_handler(const Request& request, Response& response);

//蓝牙设备注册
    int bleDeviceRegister_service_handler(const Request& request, Response& response);

//蓝牙设备操作
    int bleDeviceOperation_service_handler(const Request& request, Response& response);

//请求更新白名单
    int updateWhiteList_service_handler(const Request& request, Response& response);

//更新设备列表
    int updateDeviceList_service_handler(const Request& request, Response& response);

//更新组列表
    int updateGroupList_service_handler(const Request& request, Response& response);

//获取发现的蓝牙、zigbee站点
    int getSiteNames_service_handler(const Request& request, Response& response);



//重置网关
    int reset_device_service_handler(const Request& request, Response& response);

//扫描设备
    int scan_device_service_handler(const Request& request, Response& response);

//添加设备
    int add_device_service_handler(const Request& request, Response& response);

//删除设备
    int del_device_service_handler(const Request& request, Response& response);

//设备控制
    int deviceControl_service_handler(const Request& request, Response& response);

//获取设备列表：ble, zigbee, tvAdapter
    int getDeviceList_service_handler(const Request& request, Response& response);

//todo 获取设备状态
    int get_device_state_service_handler(const Request& request, Response& response);

//创建分组
    int create_group_service_handler(const Request& request, Response& response);

//删除分组，删除前先将设备移除分组
    int delete_group_service_handler(const Request& request, Response& response);

//重命名分组
    int rename_group_service_handler(const Request& request, Response& response);

//添加设备进入分组
    int addDevice2Group_service_handler(const Request& request, Response& response);

//从分组移除设备
    int removeDeviceFromGroup_service_handler(const Request& request, Response& response);

//组控制
    int groupControl_service_handler(const Request& request, Response& response);

//获取分组列表：ble, zigbee
    int getGroupList_service_handler(const Request& request, Response& response);
}

#endif //EXHIBITION_SERVICEREQUESTHANDLER_H
