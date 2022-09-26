//
// Created by 78472 on 2022/9/26.
//

#ifndef EXHIBITION_DEVICEMANAGER_H
#define EXHIBITION_DEVICEMANAGER_H

#include "qlibc/QData.h"
#include <atomic>

class DeviceManager {
private:
    std::atomic<bool> changed{true};
    qlibc::QData deviceList;
    DeviceManager() = default;
    static DeviceManager* instance;

public:
    static DeviceManager* getInstance();

    //列表变更
    void listChanged();

    //获取设备列表
    qlibc::QData getAllDeviceList();

private:
    //获取设备列表
    qlibc::QData getDeviceList();

    qlibc::QData addSourceTag(qlibc::QData deviceList, string sourceSite);

    qlibc::QData mergeList(qlibc::QData& ble_list, qlibc::QData& zigbeeList, qlibc::QData& tvAdapterList);
};


#endif //EXHIBITION_DEVICEMANAGER_H
