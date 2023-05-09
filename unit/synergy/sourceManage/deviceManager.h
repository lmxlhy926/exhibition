//
// Created by 78472 on 2022/9/26.
//

#ifndef EXHIBITION_DEVICEMANAGER_H
#define EXHIBITION_DEVICEMANAGER_H

#include "qlibc/QData.h"
#include <atomic>
#include <mutex>
#include <thread>
#include "siteManager.h"

class DeviceManager {
private:
    std::atomic<bool> changed{true};
    qlibc::QData deviceList_;
    std::map<string, qlibc::QData> siteDeviceListMap;   //按站点保存的设备列表
    std::recursive_mutex Mutex;
    DeviceManager(){}
    static DeviceManager* instance;

public:
    static DeviceManager* getInstance();

    //获取设备列表
    qlibc::QData getAllDeviceList();

    //判断设备是否在设备列表里
    bool isInDeviceList(string& device_id, string& inSourceSite, string& outSourceSite);

    //去掉来源标识，还原mac
    qlibc::QData restoreMac(qlibc::QData& item, string& inSourceSite);

    //更新设备列表
    void updateDeviceList();

private:
    //设备mac后增加来源标识
    qlibc::QData addMacSource(qlibc::QData deviceList, string sourceTag);

    //站点拼接
    void mergeList(qlibc::QData& list, qlibc::QData& totalList);

    //更新设备map
    void updateSiteDeviceListMap(string siteName, qlibc::QData deviceListData);
};


#endif //EXHIBITION_DEVICEMANAGER_H
