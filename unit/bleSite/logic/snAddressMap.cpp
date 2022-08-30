//
// Created by 78472 on 2022/7/20.
//

#include "snAddressMap.h"
#include "formatTrans/bleConfig.h"
#include <algorithm>
#include <vector>
#include <sstream>
#include "log/Logging.h"

SnAddressMap* SnAddressMap::instance = nullptr;

qlibc::QData SnAddressMap::getNodeAssignAddr(string deviceSn) {
    qlibc::QData nodeAddrData;
    nodeAddrData.setString("command", "assignNodeAddress");
    nodeAddrData.setString("nodeAddress",getAddress(deviceSn));
    return nodeAddrData;
}

void SnAddressMap::deleteDeviceSn(string& deviceSn) {
    std::lock_guard<recursive_mutex> lg(rMutex_);
    auto pos = snAddrMap.find(deviceSn);
    if(pos != snAddrMap.end()){
        snAddrMap.erase(pos->first);
    }
    map2JsonDataAndSave2File();
}

qlibc::QData SnAddressMap::getAddrList() {
    std::lock_guard<recursive_mutex> lg(rMutex_);
    qlibc::QData data;
    for(auto& elem : snAddrMap){
        data.setValue(elem.first, elem.second);
    }
    return data;
}

string SnAddressMap::deviceSn2Address(string deviceSn){
    std::lock_guard<recursive_mutex> lg(rMutex_);
    for(auto& elem : snAddrMap){
        if(elem.first == deviceSn){
            return elem.second["unicast_address"].asString();
        }
    }
    return "";
}

string SnAddressMap::address2DeviceSn(string address){
    std::lock_guard<recursive_mutex> lg(rMutex_);
    for(auto& elem : snAddrMap){
        if(elem.second["unicast_address"] == address){
            return elem.first;
        }
    }
    return "";
}

void SnAddressMap::loadCache2Map() {
    qlibc::QData snAddressData = bleConfig::getInstance()->getSnAddrData();
    Json::Value::Members deviceSnVec = snAddressData.getMemberNames();
    for(auto& deviceSn : deviceSnVec){
        qlibc::QData propertyItem = snAddressData.getData(deviceSn);
        snAddrMap.insert(std::make_pair(deviceSn, propertyItem.asValue()));
    }
}

void SnAddressMap::map2JsonDataAndSave2File() {
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    qlibc::QData data;
    for(auto& elem : snAddrMap){
        data.setValue(elem.first, elem.second);
    }

    bleConfig::getInstance()->saveSnAddrData(data);
}

void SnAddressMap::insert(string &deviceSn, unsigned int intAddr) {
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    auto pos = snAddrMap.find(deviceSn);
    if(pos != snAddrMap.end()){
        snAddrMap.erase(pos);
    }
    qlibc::QData propertyItem;
    propertyItem.setString("unicast_address", intAddr2FullAddr(intAddr));

    snAddrMap.insert(std::make_pair(deviceSn, propertyItem.asValue()));
}

string SnAddressMap::intAddr2FullAddr(unsigned int i) {
    stringstream ss;
    ss << "02" << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << i;
    return ss.str();
}

string SnAddressMap::getAddress(string &deviceSn) {
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    //如果设备列表为空
    if(snAddrMap.empty()){
        insert(deviceSn, 0);
        map2JsonDataAndSave2File();
        return intAddr2FullAddr(0);
    }

    //将数字地址存入vector并排序
    std::vector<int> addrVec;
    for(auto& elem : snAddrMap){
        int unicastInt = stoi(elem.second["unicast_address"].asString().substr(2, 2),
                              nullptr, 16);
        addrVec.push_back(unicastInt);
    }
    sort(addrVec.begin(), addrVec.end(), less<>());

    //如果有空隙，则返回最小空隙值
    for(unsigned int i = 0; i < addrVec.size(); ++i){
        if(i != addrVec[i]){
            insert(deviceSn, i);
            map2JsonDataAndSave2File();
            return intAddr2FullAddr(i);
        }
    }

    //没有空隙
    insert(deviceSn, addrVec.size());
    map2JsonDataAndSave2File();
    return intAddr2FullAddr(addrVec.size());
}


