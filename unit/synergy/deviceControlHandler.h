//
// Created by 78472 on 2022/5/17.
//

#ifndef EXHIBITION_DEVICECONTROLHANDLER_H
#define EXHIBITION_DEVICECONTROLHANDLER_H


#include "qlibc/QData.h"
#include <iostream>

struct controlData {
    const string deviceType;
    const string area;
    const string deviceName;
    const string command;
    const qlibc::QData param;

    explicit controlData(qlibc::QData &data) :
            deviceType(data.getData("request").getString("deviceType")),
            area(data.getData("request").getString("area")),
            deviceName(data.getData("request").getString("deviceName")),
            command(data.getData("request").getData("commands").getArrayElement(0).getString("command")),
            param(data.getData("request").getData("commands").getArrayElement(0).getData("params")){};

    void show() const{
        std::cout << "deviceType: " << deviceType << ", area: " << area << ", deviceName: " << deviceName << ", command: " << command << std::endl;
    }
};

bool deviceControlHandler(qlibc::QData& message);


#endif //EXHIBITION_DEVICECONTROLHANDLER_H
