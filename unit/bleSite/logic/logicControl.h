//
// Created by 78472 on 2022/7/11.
//

#ifndef EXHIBITION_LOGICCONTROL_H
#define EXHIBITION_LOGICCONTROL_H

#include <string>
#include <atomic>
#include "qlibc/QData.h"

using namespace std;

class LogicControl {
private:
    static atomic<bool> bindingFlag;
public:
    static bool parse(qlibc::QData& cmdData);
};


#endif //EXHIBITION_LOGICCONTROL_H
