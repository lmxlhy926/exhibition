//
// Created by 78472 on 2022/7/11.
//

#ifndef EXHIBITION_LOGICCONTROL_H
#define EXHIBITION_LOGICCONTROL_H

#include <string>
#include <atomic>
#include "qlibc/QData.h"
#include "bindDevice.h"

using namespace std;

class LogicControl {
private:
    atomic<bool> bindingFlag{false};    //标志是否处于绑定状态
    BindDevice bd;
public:
     bool parse(qlibc::QData& cmdData);

     //获取需要绑定的设备
     void getScanedDevices(qlibc::QData& deviceArray, qlibc::QData& param);
};

#endif //EXHIBITION_LOGICCONTROL_H
