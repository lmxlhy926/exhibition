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
    atomic<bool> bindingFlag{false};
    BindDevice bd;
public:
     bool parse(qlibc::QData& cmdData);

     //获取需要绑定的设备
     void getScanedDevices(qlibc::QData& deviceArray);

};


#endif //EXHIBITION_LOGICCONTROL_H
