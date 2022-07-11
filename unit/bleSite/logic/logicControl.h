//
// Created by 78472 on 2022/7/11.
//

#ifndef EXHIBITION_LOGICCONTROL_H
#define EXHIBITION_LOGICCONTROL_H

#include <string>
#include <atomic>
#include "qlibc/QData.h"

using namespace std;

//串口发送数据
bool serialSend(unsigned char *buf, int size);

//转换并发送命令
bool transAndSendCmd(qlibc::QData& controlData);


class LogicControl {
private:
    static atomic<bool> scanFlag;
public:
    static bool parse(qlibc::QData& cmdData);
};


#endif //EXHIBITION_LOGICCONTROL_H
