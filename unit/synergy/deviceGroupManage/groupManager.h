//
// Created by 78472 on 2022/9/27.
//

#ifndef EXHIBITION_GROUPMANAGER_H
#define EXHIBITION_GROUPMANAGER_H

#include <atomic>
#include "qlibc/QData.h"
#include <mutex>
#include <thread>

class GroupManager {
private:
    std::atomic<bool> changed{true};
    qlibc::QData groupList;
    std::mutex Mutex;
    std::thread* updateListThread;
    GroupManager(){
        while(true){
            updateListThread = new thread([this]{
                getAllGroupList();
                std::this_thread::sleep_for(std::chrono::seconds(5));
            });
        }
    }
    static GroupManager* instance;

public:
    static GroupManager* getInstance();

    //列表变更
    void listChanged();

    //获取设备列表
    qlibc::QData getAllGroupList();

    //是否在分组列表中
    bool isInGroupList(string& group_id, string& sourceSite);

private:
    //获取设备列表
    qlibc::QData getGroupListLocal();

    //更新站点
    void updateSite();

    //获取组列表
    qlibc::QData getGroupListAllLocalNet();

    //增加站点标识
    qlibc::QData addSourceTag(qlibc::QData deviceList, string sourceSite);

    //站点拼接
    qlibc::QData mergeList(qlibc::QData& ble_list, qlibc::QData& zigbeeList, qlibc::QData& tvAdapterList);

    //站点拼接
    void mergeList(qlibc::QData& list, qlibc::QData& totalList);

};

#endif //EXHIBITION_GROUPMANAGER_H
