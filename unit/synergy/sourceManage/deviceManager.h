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
    std::recursive_mutex Mutex;
    std::thread* updateListThread;
    const int updateListInterval = 10;
    DeviceManager(){
        updateDeviceList();
        //开启线程定时更新设备列表
        updateListThread = new thread([this]{
            while(true){
                std::this_thread::sleep_for(std::chrono::seconds(updateListInterval));
                updateDeviceList();
            }
        });
    }

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
};


#endif //EXHIBITION_DEVICEMANAGER_H
