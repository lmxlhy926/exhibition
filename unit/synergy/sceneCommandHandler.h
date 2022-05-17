//
// Created by 78472 on 2022/5/17.
//

#ifndef EXHIBITION_SCENECOMMANDHANDLER_H
#define EXHIBITION_SCENECOMMANDHANDLER_H


#include "qlibc/QData.h"


static const string ADAPTER_IP = "127.0.0.1";
static const int ADAPTER_PORT = 60003;

struct controlData {
    const string deviceType;
    const string area;
    const string deviceName;
    const string command;

    explicit controlData(qlibc::QData &data) :
            deviceType(data.getData("param").getString("deviceType")),
            area(data.getData("param").getString("area")),
            deviceName(data.getData("param").getString("deviceName")),
            command(data.getData("param").getData("commands").getArrayElement(0).getString("command")) {};


};

bool deviceControlHandler(const string& uri, qlibc::QData& message);


#endif //EXHIBITION_SCENECOMMANDHANDLER_H
