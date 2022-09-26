//
// Created by 78472 on 2022/9/26.
//

#include "deviceManager.h"
#include "common/httpUtil.h"
#include "param.h"

DeviceManager* DeviceManager::instance = nullptr;

DeviceManager *DeviceManager::getInstance() {
    if(instance == nullptr){
        instance = new DeviceManager();
    }
    return instance;
}

void DeviceManager::listChanged() {
    changed.store(true);
}

qlibc::QData DeviceManager::getAllDeviceList() {
    if(changed.load()){
        deviceList = getDeviceList();
    }
    changed.store(false);
    return deviceList;
}

qlibc::QData DeviceManager::getDeviceList(){
    qlibc::QData deviceRequest;
    deviceRequest.setString("service_id", "get_device_list");
    deviceRequest.setValue("request", Json::nullValue);

    qlibc::QData bleDeviceRes, zigbeeDeviceRes, tvDeviceRes;
    SiteRecord::getInstance()->sendRequest2Site(BleSiteID, deviceRequest, bleDeviceRes);
    SiteRecord::getInstance()->sendRequest2Site(ZigbeeSiteID, deviceRequest, zigbeeDeviceRes);
    SiteRecord::getInstance()->sendRequest2Site(TvAdapterSiteID, deviceRequest, tvDeviceRes);

    qlibc::QData ble_list = addSourceTag(bleDeviceRes.getData("response").getData("device_list"), BleSiteID);
    qlibc::QData zigbee_list = addSourceTag(zigbeeDeviceRes.getData("response").getData("device_list"), ZigbeeSiteID);
    qlibc::QData tvAdapterList = addSourceTag(tvDeviceRes.getData("response").getData("device_list"), TvAdapterSiteID);

    mergeList(ble_list, zigbee_list, tvAdapterList);
    return mergeList(ble_list, zigbee_list, tvAdapterList);
}

qlibc::QData DeviceManager::addSourceTag(qlibc::QData deviceList, string sourceSite){
    Json::ArrayIndex num = deviceList.size();
    qlibc::QData newDeviceList;
    for(Json::ArrayIndex i = 0; i < num; ++i){
        qlibc::QData item = deviceList.getArrayElement(i);
        item.setString("sourceSite", sourceSite);
        newDeviceList.append(item);
    }
    return newDeviceList;
}

qlibc::QData DeviceManager::mergeList(qlibc::QData& ble_list, qlibc::QData& zigbeeList, qlibc::QData& tvAdapterList){
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





