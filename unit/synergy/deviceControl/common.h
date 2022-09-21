//
// Created by 78472 on 2022/6/2.
//

#ifndef EXHIBITION_COMMON_H
#define EXHIBITION_COMMON_H

#include <string>
#include <iostream>
#include "qlibc/QData.h"

using namespace std;

class DownCommandData {
private:
     string deviceType;         //设备类型
     string area;               //设备所在区域
     string deviceName;         //设备名称
     qlibc::QData command_list;      //设备操作命令

public:
    explicit DownCommandData(qlibc::QData &data) {
        qlibc::QData request = data.getData("request");
        deviceType = request.getString("deviceType"),
        area = request.getString("area"),
        deviceName = request.getString("deviceName"),
        command_list = request.getData("commands");
    }

    qlibc::QData getContorlData(qlibc::QData& deviceList);

    bool match(qlibc::QData& item);
};



class ControlBase{
protected:
    //控制命令中要控制的设备是否为该设备数据项
    virtual bool match(const DownCommandData &downCommand, qlibc::QData &deviceItem) = 0;

    //构造智慧家Adapter站点的设备控制指令
    virtual qlibc::QData constructCtrCmd(const DownCommandData &downCommand, qlibc::QData &deviceItem) = 0;

    //封装命令
    virtual qlibc::QData encapsulate(qlibc::QData& command, qlibc::QData& deviceItem);

public:
    //请求站点服务
    static bool sitePostRequest(const string& ip, int port, qlibc::QData& request, qlibc::QData& response);
};

#endif //EXHIBITION_COMMON_H
