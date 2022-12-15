//
// Created by 78472 on 2022/9/27.
//

#ifndef EXHIBITION_GROUPMANAGER_H
#define EXHIBITION_GROUPMANAGER_H

#include <atomic>
#include "qlibc/QData.h"

class GroupManager {
private:
    std::atomic<bool> changed{true};
    qlibc::QData groupList;
    GroupManager() = default;
    static GroupManager* instance;

public:
    static GroupManager* getInstance();

    //列表变更
    void listChanged();

    //获取设备列表
    qlibc::QData getAllGroupList();

private:
    //获取设备列表
    qlibc::QData getGroupList();

    qlibc::QData addSourceTag(qlibc::QData deviceList, string sourceSite);

    qlibc::QData mergeList(qlibc::QData& ble_list, qlibc::QData& zigbeeList, qlibc::QData& tvAdapterList);
};

#endif //EXHIBITION_GROUPMANAGER_H
