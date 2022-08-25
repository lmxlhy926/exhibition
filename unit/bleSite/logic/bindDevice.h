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
    bool addDevice(string& deviceSn);

    //将新增设备插入设备列表
    void insert2DeviceList(string& deviceID);

    //对新增设备插入默认状态
    void insertDefaultStatus(string& deviceID);

    //产生设备的默认状态
    qlibc::QData defaultStatus();
};

#endif //EXHIBITION_BINDDEVICE_H
