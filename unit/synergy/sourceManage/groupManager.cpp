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

qlibc::QData GroupManager::getAllGroupList() {
    std::lock_guard<std::mutex> lg(Mutex);
    return groupList;
}

qlibc::QData GroupManager::getBleGroupList(){
    qlibc::QData allGroupList = getAllGroupList();
    qlibc::QData bleGroupList;
    Json::ArrayIndex size = allGroupList.size();
    for(Json::ArrayIndex i = 0; i < size; ++i){
        qlibc::QData item = allGroupList.getArrayElement(i);
        string sourceSite = item.getString("sourceSite");
        if(regex_match(sourceSite, regex(".*:ble_light"))){
            bleGroupList.append(item);
        }
    }
    return bleGroupList;
}

bool GroupManager::isInGroupList(string& group_id, string& inSourceSite, string& outSourceSite){
    std::lock_guard<std::mutex> lg(Mutex);
    Json::ArrayIndex size = groupList.size();
    for(Json::ArrayIndex i = 0; i < size; ++i){
        qlibc::QData item = groupList.getArrayElement(i);
        string itemGroupUid = item.getString("group_uid");
        string groupUid = group_id;
        if(!inSourceSite.empty()){
            groupUid.append(">").append(inSourceSite);
        }

        if(itemGroupUid == groupUid){
            smatch sm;
            if(regex_match(itemGroupUid, sm, regex("(.*)>(.*)"))){
                outSourceSite = sm.str(2);
                return true;
            }
        }
    }
    return false;
}

qlibc::QData GroupManager::restoreGrp(qlibc::QData& item, string& inSourceSite){
    if(inSourceSite.empty()){
        string group_id = item.getString("group_id");
        smatch sm;
        if(regex_match(group_id, sm, regex("(.*)>(.*)"))){
            group_id = sm.str(1);
            item.setString("group_id", group_id);
        }
    }
    item.removeMember("sourceSite");
    return item;
}

void GroupManager::updateGroupList(){
    qlibc::QData totalList;
    std::set<string> siteNameSet = SiteRecord::getInstance()->getSiteName();
    smatch sm;
    for(auto& elem : siteNameSet){
        if(regex_match(elem, sm, regex("(.*):(.*)"))){
            string uid = sm.str(1);
            string siteID = sm.str(2);
            if((siteID == BleSiteID || siteID == ZigbeeSiteID)){
                qlibc::QData groupRequest, groupRes;
                groupRequest.setString("service_id", "get_group_list");
                groupRequest.setValue("request", Json::nullValue);
                SiteRecord::getInstance()->sendRequest2Site(sm.str(0), groupRequest, groupRes);     //获取组列表
                qlibc::QData list = addGrpSourceTag(groupRes.getData("response").getData("group_list"),
                                                 string().append(uid).append(":").append(siteID));    //给组条目添加标签
                mergeList(list, totalList);
            }
        }
    }

    std::lock_guard<std::mutex> lg(Mutex);
    groupList = totalList;
    init.store(true);
}

qlibc::QData GroupManager::addGrpSourceTag(qlibc::QData groupList, string grpSource){
    Json::ArrayIndex num = groupList.size();
    qlibc::QData newGroupList;
    for(Json::ArrayIndex i = 0; i < num; ++i){
        qlibc::QData item = groupList.getArrayElement(i);
        string group_id = item.getString("group_id");
        group_id.append(">").append(grpSource);
        item.setString("sourceSite", grpSource);
        item.setString("group_uid", group_id);
        newGroupList.append(item);
    }
    return newGroupList;
}

void GroupManager::mergeList(qlibc::QData &list, qlibc::QData &totalList) {
    for(Json::ArrayIndex i = 0; i < list.size(); ++i){
        qlibc::QData item = list.getArrayElement(i);
        totalList.append(item);
    }
}
