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
public:
    const string deviceType;    //设备类型
    const string area;          //设备所在区域
    const string deviceName;    //设备名称
    const string command;       //设备操作命令
    const qlibc::QData param;   //操作命令携带的参数

    explicit DownCommandData(qlibc::QData &data) :
            deviceType(data.getData("request").getString("deviceType")),
            area(data.getData("request").getString("area")),
            deviceName(data.getData("request").getString("deviceName")),
            command(data.getData("request").getData("commands").getArrayElement(0).getString("command")),
            param(data.getData("request").getData("commands").getArrayElement(0).getData("params")){};

    void show() const{
        std::cout << "deviceType: " << deviceType << ", area: " << area << ", deviceName: " << deviceName << ", command: " << command << std::endl;
    }
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
