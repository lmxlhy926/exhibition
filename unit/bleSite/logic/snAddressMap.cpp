//
// Created by 78472 on 2022/7/20.
//

#include "snAddressMap.h"
#include "formatTrans/bleConfig.h"
#include <algorithm>
#include <vector>
#include <sstream>

qlibc::QData SnAddressMap::getNodeAssignAddr(string deviceSn) {
    std::lock_guard<recursive_mutex> lg(mutex_);
    qlibc::QData nodeAddrData;
    nodeAddrData.setString("command", "assignNodeAddress");
    nodeAddrData.setString("nodeAddress",getAddress(deviceSn));
    return nodeAddrData;
}

void SnAddressMap::deleteDeviceSn(string& deviceSn) {
    std::lock_guard<recursive_mutex> lg(mutex_);
    auto pos = snAddrMap.find(deviceSn);
    if(pos != snAddrMap.end()){
        snAddrMap.erase(pos->first);
    }
    map2JsonDataAndSave2File();
}

qlibc::QData SnAddressMap::getDeviceList() {
    std::lock_guard<recursive_mutex> lg(mutex_);
    qlibc::QData data;
    for(auto& elem : snAddrMap){
        data.setString(elem.first, intAddr2FullAddr(elem.second));
    }
    return data;
}


string SnAddressMap::deviceSn2Address(string deviceSn){
    std::lock_guard<recursive_mutex> lg(mutex_);
    for(auto& elem : snAddrMap){
        if(elem.first == deviceSn){
            return intAddr2FullAddr(elem.second);
        }
    }
    return "";
}

void SnAddressMap::loadCache2Map() {
    qlibc::QData snAddressData = bleConfig::getInstance()->getSnAddrData();
    Json::Value::Members deviceSnVec = snAddressData.getMemberNames();
    for(auto& deviceSn : deviceSnVec){
        int i = snAddressData.getData(deviceSn).getArrayElement(1).asValue().asInt();
        snAddrMap.insert(std::make_pair(deviceSn, i));
    }
}

void SnAddressMap::insert(string &deviceSn, unsigned int intAddr) {
    auto pos = snAddrMap.find(deviceSn);
    if(pos != snAddrMap.end()){
        snAddrMap.erase(pos);
    }
    snAddrMap.insert(std::make_pair(deviceSn, intAddr));
}

string SnAddressMap::intAddr2FullAddr(unsigned int i) {
    stringstream ss;
    ss << "02" << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << i;
    return ss.str();
}

void SnAddressMap::map2JsonDataAndSave2File() {
    qlibc::QData data;
    for(auto& elem : snAddrMap){
        qlibc::QData array;
        array.append(intAddr2FullAddr(elem.second));
        array.append(elem.second);

        data.putData(elem.first, array);
    }
    bleConfig::getInstance()->saveSnAddrData(data);
}


string SnAddressMap::getAddress(string &deviceSn) {
    //如果设备列表为空
    if(snAddrMap.empty()){
        insert(deviceSn, 0);
        map2JsonDataAndSave2File();
        return intAddr2FullAddr(0);
    }

    //将数字地址存入vector并排序
    std::vector<int> addrVec;
    for(auto& elem : snAddrMap){
        addrVec.push_back(elem.second);
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











