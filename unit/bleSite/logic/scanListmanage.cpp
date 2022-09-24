//
// Created by 78472 on 2022/9/24.
//

#include "scanListmanage.h"
#include "formatTrans/bleConfig.h"

ScanListmanage* ScanListmanage::instance = nullptr;

ScanListmanage::ScanListmanage() {
    loadData();
}

ScanListmanage *ScanListmanage::getInstance() {
    if(instance == nullptr){
        instance = new ScanListmanage;
    }
    return instance;
}

void ScanListmanage::appendDeviceItem(string deviceSn, Json::Value property) {
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    auto pos = list.find(deviceSn);
    if(pos != list.end()){
        list.erase(pos);
    }
    list.insert(std::make_pair(deviceSn, property));
    saveData();
}

void ScanListmanage::deleteDeviceItem(string &deviceSn) {
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    auto pos = list.find(deviceSn);
    if(pos != list.end()){
        list.erase(pos);
    }
    saveData();
}

std::map<string, Json::Value> ScanListmanage::getScanListMap() {
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    return list;
}

void ScanListmanage::loadData() {
    qlibc::QData data = bleConfig::getInstance()->getScanListData();
    ssize_t num = data.size();
    for(Json::ArrayIndex i = 0; i < num; ++i){
        qlibc::QData item = data.getArrayElement(i);
        appendDeviceItem(item.getString("deviceSn"), item.asValue());
    }
}

void ScanListmanage::saveData() {
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    qlibc::QData saveData;
    for(auto & pos : list){
        saveData.append(pos.second);
    }
    bleConfig::getInstance()->saveScanListData(saveData);
}


