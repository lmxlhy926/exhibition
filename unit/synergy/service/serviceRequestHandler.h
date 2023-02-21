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

//获取设备列表：ble, zigbee, tvAdapter
    int getDeviceList_service_handler(const Request& request, Response& response);

//获取分组列表：ble, zigbee
    int getGroupList_service_handler(const Request& request, Response& response);

//语音控制入口
    int voiceControl_service_handler(const Request& request, Response& response);

//设备控制
    int deviceControl_service_handler(const Request& request, Response& response);

//组控制
    int groupControl_service_handler(const Request& request, Response& response);

//蓝牙设备注册
    int bleDeviceRegister_service_handler(const Request& request, Response& response);

//蓝牙设备操作
    int bleDeviceOperation_service_handler(const Request& request, Response& response);

}

#endif //EXHIBITION_SERVICEREQUESTHANDLER_H
