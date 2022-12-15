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

    groupList = getGroupList();
    return groupList;
}

qlibc::QData GroupManager::getGroupList(){
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