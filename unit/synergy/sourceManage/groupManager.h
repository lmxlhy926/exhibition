//
// Created by 78472 on 2022/9/27.
//

#ifndef EXHIBITION_GROUPMANAGER_H
#define EXHIBITION_GROUPMANAGER_H

#include <atomic>
#include "qlibc/QData.h"
#include <mutex>
#include <thread>
#include "siteManager.h"

class GroupManager {
private:
    std::atomic<bool> init{false};
    qlibc::QData groupList;
    std::mutex Mutex;
    GroupManager(){}
    static GroupManager* instance;

public:
    static GroupManager* getInstance();

    //获取组列表
    qlibc::QData getAllGroupList();

    //获取蓝牙组列表
    qlibc::QData getBleGroupList();

    //是否在组列表中
    bool isInGroupList(string& group_id, string& inSourceSite, string& outSourceSite);

    //去掉来源标识，还原组
    qlibc::QData restoreGrp(qlibc::QData& item, string& inSourceSite);

    //更新组列表
    void updateGroupList();

public:
    //增加组来源标识
    qlibc::QData addGrpSourceTag(qlibc::QData groupList, string grpSource);

    //站点拼接
    void mergeList(qlibc::QData& list, qlibc::QData& totalList);
};

#endif //EXHIBITION_GROUPMANAGER_H
