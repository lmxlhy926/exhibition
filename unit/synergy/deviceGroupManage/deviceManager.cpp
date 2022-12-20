//
// Created by 78472 on 2022/9/26.
//

#include "deviceManager.h"
#include "common/httpUtil.h"
#include "../param.h"

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
//    if(changed.load()){
//        deviceList = getDeviceList();
//    }
//    changed.store(false);

    qlibc::QData list = getDeviceListAllLocalNet();
    std::lock_guard<std::mutex> lg(Mutex);
    deviceList_ = list;
    return deviceList_;
}

bool DeviceManager::isInDeviceList(string& device_id, string& sourceSite){
    std::lock_guard<std::mutex> lg(Mutex);
    Json::ArrayIndex size = deviceList_.size();
    for(Json::ArrayIndex i = 0; i < size; ++i){
        qlibc::QData item = deviceList_.getArrayElement(i);
        if(item.getString("device_id") == device_id){
            sourceSite = item.getString("sourceSite");
            return true;
        }
    }
    return false;
}

qlibc::QData DeviceManager::getDeviceListLocal(){
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

    return mergeList(ble_list, zigbee_list, tvAdapterList);
}

void DeviceManager::updateSite(){
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
                if(site_id == BleSiteID || site_id == TvAdapterSiteID){
                    string ipSiteName = elem + ":" + site_id;
                    SiteRecord::getInstance()->addSite(ipSiteName, elem, item.getInt("port"));
                }
            }
        }
    }
}

qlibc::QData DeviceManager::getDeviceListAllLocalNet() {
    updateSite();
    qlibc::QData totalList;
    std::set<string> siteNameSet = SiteRecord::getInstance()->getSiteName();
    smatch sm;
    for(auto& elem : siteNameSet){
        if(regex_match(elem, sm, regex("(.*):(.*)"))){
            string ip = sm.str(1);
            string siteID = sm.str(2);
            if(siteID == BleSiteID || siteID == TvAdapterSiteID){
                qlibc::QData deviceRequest;
                deviceRequest.setString("service_id", "get_device_list");
                deviceRequest.setValue("request", Json::nullValue);
                qlibc::QData deviceRes;
                SiteRecord::getInstance()->sendRequest2Site(sm.str(0), deviceRequest, deviceRes);
                qlibc::QData list = addSourceTag(deviceRes.getData("response").getData("device_list"), sm.str(0));
                mergeList(list, totalList);
            }
        }
    }
    return totalList;
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


void DeviceManager::mergeList(qlibc::QData &list, qlibc::QData &totalList) {
    for(Json::ArrayIndex i = 0; i < list.size(); ++i){
        qlibc::QData item = list.getArrayElement(i);
        totalList.append(item);
    }
}





