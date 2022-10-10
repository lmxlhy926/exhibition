//
// Created by 78472 on 2022/6/2.
//

#ifndef EXHIBITION_DOWNCOMMAND_H
#define EXHIBITION_DOWNCOMMAND_H

#include <string>
#include <iostream>
#include "qlibc/QData.h"

using namespace std;

class DownCommandData {
private:
     string type;               //控制对象类型
     string action;             //控制动作
     string code;               //控制设备标识
     string delay;              //延时
     qlibc::QData inParams;     //控制参数

public:
    explicit DownCommandData(qlibc::QData &data) {
        qlibc::QData request = data.getData("request");
        type = request.getString("type");
        action = request.getString("action");
        code = request.getString("code");
        delay = request.getString("delay");
        inParams = request.getData("inParams");
    }

    qlibc::QData getContorlData(qlibc::QData& deviceList);

    bool match(qlibc::QData& item);

    qlibc::QData buildCommandList(qlibc::QData& data);

    bool commandMatch(string& key);
};



#endif //EXHIBITION_DOWNCOMMAND_H