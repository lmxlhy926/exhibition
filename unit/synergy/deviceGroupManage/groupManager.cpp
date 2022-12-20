//
// Created by 78472 on 2022/9/27.
//

#include "groupManager.h"
#include "common/httpUtil.h"
#include "../param.h"

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
//    if(changed.load()){
//        groupList = getGroupList();
//    }
//    changed.store(false);

    qlibc::QData list = getGroupListAllLocalNet();
    std::lock_guard<std::mutex> lg(Mutex);
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

qlibc::QData GroupManager::getGroupListLocal(){
    qlibc::QData deviceRequest;
    deviceRequest.setString("service_id", "get_group_list");
    deviceRequest.setValue("request", Json::nullValue);

    qlibc::QData bleDeviceRes, zigbeeDeviceRes, tvDeviceRes;
    SiteRecord::getInstance()->sendRequest2Site(BleSiteID, deviceRequest, bleDeviceRes);
    SiteRecord::getInstance()->sendRequest2Site(ZigbeeSiteID, deviceRequest, zigbeeDeviceRes);
    SiteRecord::getInstance()->sendRequest2Site(TvAdapterSiteID, deviceRequest, tvDeviceRes);

    qlibc::QData ble_list = addSourceTag(bleDeviceRes.getData("response").getData("group_list"), BleSiteID);
    qlibc::QData zigbee_list = addSourceTag(zigbeeDeviceRes.getData("response").getData("group_list"), ZigbeeSiteID);
    qlibc::QData tvAdapterList = addSourceTag(tvDeviceRes.getData("response").getData("group_list"), TvAdapterSiteID);

    return mergeList(ble_list, zigbee_list, tvAdapterList);
}

void GroupManager::updateSite(){
    qlibc::QData request, response;
    request.setString("service_id", "site_localAreaNetworkSite");
    request.putData("request", qlibc::QData().setString("site_id", ""));

    if(httpUtil::sitePostRequest("127.0.0.1", 9000, request, response)){
        qlibc::QData resBody = response.getData("response");
        Json::Value::Members members = resBody.getMemberNames();
        for(auto& elem : members){
            qlibc::QData siteList = resBody.getData(elem);
            Json::ArrayIndex size = siteList.size();
            for(Json::ArrayIndex i = 0; i < size; ++i){
                qlibc::QData item = siteList.getArrayElement(i);
                string site_id = item.getString("site_id");
                if(site_id == BleSiteID || site_id == ZigbeeSiteID){
                    string ipSiteName = elem + ":" + site_id;
                    SiteRecord::getInstance()->addSite(ipSiteName, elem, item.getInt("port"));
                }
            }
        }
    }
}

qlibc::QData GroupManager::getGroupListAllLocalNet(){
    updateSite();
    qlibc::QData totalList;
    std::set<string> siteNameSet = SiteRecord::getInstance()->getSiteName();
    smatch sm;
    for(auto& elem : siteNameSet){
        if(regex_match(elem, sm, regex("(.*):(.*)"))){
            string ip = sm.str(1);
            string siteID = sm.str(2);
            if(siteID == BleSiteID || siteID == ZigbeeSiteID){
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

qlibc::QData GroupManager::mergeList(qlibc::QData& ble_list, qlibc::QData& zigbeeList, qlibc::QData& tvAdapterList){
    qlibc::QData deviceList;
    for(Json::ArrayIndex i = 0; i < ble_list.size(); ++i){
        qlibc::QData item = ble_list.getArrayElement(i);
        deviceList.append(item);
    }
    for(Json::ArrayIndex i = 0; i < zigbeeList.size(); ++i){
        qlibc::QData item = zigbeeList.getArrayElement(i);
        deviceList.append(item);
    }
    for(Json::ArrayIndex i = 0; i < tvAdapterList.size(); ++i){
        qlibc::QData item = tvAdapterList.getArrayElement(i);
        deviceList.append(item);
    }
    return deviceList;
}

void GroupManager::mergeList(qlibc::QData &list, qlibc::QData &totalList) {
    for(Json::ArrayIndex i = 0; i < list.size(); ++i){
        qlibc::QData item = list.getArrayElement(i);
        totalList.append(item);
    }
}
