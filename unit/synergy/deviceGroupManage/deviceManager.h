//
// Created by 78472 on 2022/9/26.
//

#ifndef EXHIBITION_DEVICEMANAGER_H
#define EXHIBITION_DEVICEMANAGER_H

#include "qlibc/QData.h"
#include <atomic>
#include <mutex>
#include <thread>

class DeviceManager {
private:
    std::atomic<bool> changed{true};
    qlibc::QData deviceList_;
    std::mutex Mutex;
    std::thread* updateListThread;
    DeviceManager(){
        updateListThread = new thread([this]{
            while(true){
                getAllDeviceList();
                std::this_thread::sleep_for(std::chrono::seconds(5));
            }
        });
    }
    static DeviceManager* instance;

public:
    static DeviceManager* getInstance();

    //列表变更
    void listChanged();

    //获取设备列表
    qlibc::QData getAllDeviceList();

    bool isInDeviceList(string& device_id, string& sourceSite);

private:
    //获取设备列表
    qlibc::QData getDeviceListLocal();

    //更新站点
    void updateSite();

    //获取设备列表
    qlibc::QData getDeviceListAllLocalNet();

    //增加站点标识
    qlibc::QData addSourceTag(qlibc::QData deviceList, string sourceSite);

    //列表拼接
    qlibc::QData mergeList(qlibc::QData& ble_list, qlibc::QData& zigbeeList, qlibc::QData& tvAdapterList);

    //站点拼接
    void mergeList(qlibc::QData& list, qlibc::QData& totalList);
};


#endif //EXHIBITION_DEVICEMANAGER_H
