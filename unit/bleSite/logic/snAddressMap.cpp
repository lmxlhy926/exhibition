//
// Created by 78472 on 2022/7/20.
//

#include "snAddressMap.h"
#include "formatTrans/bleConfig.h"
#include <algorithm>
#include <vector>
#include <sstream>

qlibc::QData SnAddressMap::getNodeAssignAddr(string deviceSn) {
    int i = getAddrInt(deviceSn);
    stringstream ss;
    ss << "02" << std::setw(2) << std::setfill('0') << std::hex << i;
    qlibc::QData nodeAddrData;
    nodeAddrData.setString("command", "assignNodeAddress");
    nodeAddrData.setString("nodeAddress", ss.str());
    return nodeAddrData;
}

void SnAddressMap::init() {
    qlibc::QData snAddressData = bleConfig::getInstance()->getSnAddrData();
    Json::Value::Members deviceSnVec = snAddressData.asValue().getMemberNames();
    for(auto& deviceSn : deviceSnVec){
        snAddrMap.insert(std::make_pair(deviceSn, snAddressData.getInt(deviceSn)));
    }
}

void SnAddressMap::save2File() {
    qlibc::QData data;
    for(auto& elem : snAddrMap){
        data.setInt(elem.first, elem.second);
    }
    bleConfig::getInstance()->saveSnAddrData(data);
}


unsigned int SnAddressMap::getAddrInt(string &deviceSn) {
    //如果设备列表为空
    if(snAddrMap.empty()){
        snAddrMap.insert(std::make_pair(deviceSn, 00));
        save2File();
        return 0;
    }

    std::vector<int> addrVec;
    for(auto& elem : snAddrMap){
        addrVec.push_back(elem.second);
    }
    sort(addrVec.begin(), addrVec.end(), less<>());

    for(int i = 0; i < addrVec.size(); ++i){
        if(i != addrVec[i]){
            snAddrMap.insert(std::make_pair(deviceSn, i));
            save2File();
            return i;
        }
        if(i == addrVec.size() - 1){
            snAddrMap.insert(std::make_pair(deviceSn, addrVec.size()));
            save2File();
            return static_cast<unsigned int>(addrVec.size());
        }
    }
}




