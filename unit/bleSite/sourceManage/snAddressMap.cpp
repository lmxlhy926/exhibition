//
// Created by 78472 on 2022/7/20.
//

#include "snAddressMap.h"
#include "bleConfig.h"
#include <algorithm>
#include <vector>
#include <sstream>
#include "log/Logging.h"

SnAddressMap* SnAddressMap::instance = nullptr;

qlibc::QData SnAddressMap::getNodeAssignAddr(string deviceSn, uint forward) {
    qlibc::QData nodeAddrData;
    nodeAddrData.setString("command", "assignNodeAddress");
    nodeAddrData.setString("nodeAddress", getAddress(deviceSn, forward));
    return nodeAddrData;
}

void SnAddressMap::deleteDeviceSn(string& deviceSn) {
    std::lock_guard<recursive_mutex> lg(rMutex_);
    auto pos = snAddrMap.find(deviceSn);
    if(pos != snAddrMap.end()){
        snAddrMap.erase(pos->first);
        map2JsonDataAndSave2File();
    }
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
    initIndex();
}

void SnAddressMap::map2JsonDataAndSave2File() {
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    qlibc::QData data;
    for(auto& elem : snAddrMap){
        data.setValue(elem.first, elem.second);
    }
    bleConfig::getInstance()->saveSnAddrData(data);
}

void SnAddressMap::insert(string &deviceSn, string address) {
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    auto pos = snAddrMap.find(deviceSn);
    if(pos != snAddrMap.end()){
        snAddrMap.erase(pos);
    }
    qlibc::QData propertyItem;
    propertyItem.setString("unicast_address", address);
    snAddrMap.insert(std::make_pair(deviceSn, propertyItem.asValue()));
}

//从加载的sn信息中得到Index;
void SnAddressMap::initIndex(){
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    std::vector<int> highByteVec, lowByteVec;
    for(auto& elem : snAddrMap){
        try{
            string unicast_address = elem.second["unicast_address"].asString();
            uint highByte = stol(unicast_address.substr(2, 2), nullptr, 16);
            uint lowByte = stol(unicast_address.substr(0, 2), nullptr, 16);
            highByteVec.push_back(highByte);
            lowByteVec.push_back(lowByte);
        }catch(const exception& e){
            LOG_RED << "can no load snAddress, transError in initIndex: " << e.what();
            exit(-1);
        }
    }

    uint highByte{0}, lowByte{0};
    if(!highByteVec.empty()){
        highByte = *std::max_element(highByteVec.begin(), highByteVec.end());
    }
    if(!lowByteVec.empty()){
        lowByte = *std::max_element(lowByteVec.begin(), lowByteVec.end());
    }
    _index = highByte * 256 + lowByte + 1;  //可用的最小index
}

//index转换为地址
string SnAddressMap::index2Address(){
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    uint lowByte = _index % 256;
    uint highByte = _index / 256;
    stringstream ss;
    ss << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << lowByte;
    ss << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << highByte;
    return ss.str();
}

//index步进
void SnAddressMap::indexForward(uint forward){
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    _index += forward;
}

//获取地址，步进地址空间
string SnAddressMap::getAddress(string& deviceSn, uint forward){
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    uint index = _index;  //获取索引
    string deviceSnAddress = index2Address();   //索引转换为地址
    indexForward(forward);  //改变索引
    insert(deviceSn, deviceSnAddress);  //存储地址
    map2JsonDataAndSave2File();     //存储到文件
    return deviceSnAddress; //返回地址
}


