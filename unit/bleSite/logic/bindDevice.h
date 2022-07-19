//
// Created by 78472 on 2022/7/19.
//

#ifndef EXHIBITION_BINDDEVICE_H
#define EXHIBITION_BINDDEVICE_H

#include "qlibc/QData.h"

class BindDevice {
private:
    qlibc::QData deviceArray_;
public:
    explicit BindDevice(const qlibc::QData& deviceArray) : deviceArray_(deviceArray){}

    void operator()();
};


#endif //EXHIBITION_BINDDEVICE_H
