//
// Created by 78472 on 2022/10/9.
//

#ifndef EXHIBITION_SCENECOMMAND_H
#define EXHIBITION_SCENECOMMAND_H

#include <string>
#include "qlibc/QData.h"

class SceneCommand {
private:
    string type;               //控制对象类型
    string action;             //控制动作
    string code;               //控制设备标识
    string delay;              //延时
    qlibc::QData inParams;     //控制参数

public:
    explicit SceneCommand(qlibc::QData &data) {
        qlibc::QData request = data.getData("request");
        type = request.getString("type");
        action = request.getString("action");
        code = request.getString("code");
        delay = request.getString("delay");
        inParams = request.getData("inParams");
    }

    bool sendCmd(qlibc::QData& siteResponse);
};


#endif //EXHIBITION_SCENECOMMAND_H
