//
// Created by 78472 on 2022/9/27.
//

#include "groupManager.h"
#include "common/httpUtil.h"
#include "../param.h"
#include "log/Logging.h"

GroupManager* GroupManager::instance = nullptr;

GroupManager *GroupManager::getInstance() {
    if(instance == nullptr){
        instance = new GroupManager();
    }
    return instance;
}

void GroupManager::listChanged() {
    changed.store(true);
}

qlibc::QData GroupManager::getAllGroupList() {
    std::lock_guard<std::mutex> lg(Mutex);
    qlibc::QData list = getGroupListAllLocalNet();
    groupList = list;
    return groupList;
}

bool GroupManager::isInGroupList(string& group_id, string& sourceSite){
    std::lock_guard<std::mutex> lg(Mutex);
    Json::ArrayIndex size = groupList.size();
    for(Json::ArrayIndex i = 0; i < size; ++i){
        qlibc::QData item = groupList.getArrayElement(i);
        if(item.getString("group_id") == group_id){
            sourceSite = item.getString("sourceSite");
            return true;
        }
    }
    return false;
}

qlibc::QData GroupManager::getGroupListAllLocalNet(){
    qlibc::QData totalList;
    std::set<string> siteNameSet = SiteRecord::getInstance()->getSiteName();
    smatch sm;
    for(auto& elem : siteNameSet){
        if(regex_match(elem, sm, regex("(.*):(.*)"))){
            string ip = sm.str(1);
            string siteID = sm.str(2);
            if((siteID == BleSiteID || siteID == ZigbeeSiteID) && ip != "127.0.0.1"){
                qlibc::QData groupRequest;
                groupRequest.setString("service_id", "get_group_list");
                groupRequest.setValue("request", Json::nullValue);
                qlibc::QData groupRes;
                SiteRecord::getInstance()->sendRequest2Site(sm.str(0), groupRequest, groupRes);
                qlibc::QData list = addSourceTag(groupRes.getData("response").getData("group_list"), sm.str(0));
                mergeList(list, totalList);
            }
        }
    }
    return totalList;
}

qlibc::QData GroupManager::addSourceTag(qlibc::QData deviceList, string sourceSite){
    Json::ArrayIndex num = deviceList.size();
    qlibc::QData newDeviceList;
    for(Json::ArrayIndex i = 0; i < num; ++i){
        qlibc::QData item = deviceList.getArrayElement(i);
        item.setString("sourceSite", sourceSite);
        newDeviceList.append(item);
    }
    return newDeviceList;
}

void GroupManager::mergeList(qlibc::QData &list, qlibc::QData &totalList) {
    for(Json::ArrayIndex i = 0; i < list.size(); ++i){
        qlibc::QData item = list.getArrayElement(i);
        totalList.append(item);
    }
}
