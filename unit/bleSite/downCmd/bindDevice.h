//
// Created by 78472 on 2022/7/19.
//

#ifndef EXHIBITION_BINDDEVICE_H
#define EXHIBITION_BINDDEVICE_H

#include "qlibc/QData.h"
#include "sourceManage/snAddressMap.h"

class BindDevice {
private:
    std::mutex mutex_;

public:
    BindDevice()= default;

    //绑定设备，更新配置站点白名单
    void bind(qlibc::QData& deviceArray);

private:
    //绑定设备
    bool addDevice(string& deviceSn, qlibc::QData& property);

    //更新白名单
    void updateDeviceList2ConfigSite();
};

#endif //EXHIBITION_BINDDEVICE_H
