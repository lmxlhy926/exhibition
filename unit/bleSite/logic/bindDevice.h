//
// Created by 78472 on 2022/7/19.
//

#ifndef EXHIBITION_BINDDEVICE_H
#define EXHIBITION_BINDDEVICE_H

#include "qlibc/QData.h"
#include "snAddressMap.h"

class BindDevice {
private:
    std::mutex mutex_;

public:
    BindDevice()= default;

    void bind(qlibc::QData& deviceArray);

private:
    bool addDevice(string& deviceSn, qlibc::QData& property);

    void updateDeviceList2ConfigSite();
};

#endif //EXHIBITION_BINDDEVICE_H
