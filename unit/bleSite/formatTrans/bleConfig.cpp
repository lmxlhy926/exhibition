//
// Created by 78472 on 2022/6/7.
//

#include "bleConfig.h"
#include "qlibc/FileUtils.h"
#include <iostream>
#include "log/Logging.h"

bleConfig* bleConfig::instance = nullptr;

bleConfig* bleConfig::getInstance() {
    if(instance == nullptr)
        instance = new bleConfig(10);
    return instance;
}

void bleConfig::setConfigPath(const string& configPath) {
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    dataDirPath = configPath;
}

string bleConfig::getconfigPath() {
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    return dataDirPath;
}

QData bleConfig::getBleParamData() {
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    if(bleParamData.empty()){
        bleParamData.loadFromFile(FileUtils::contactFileName(dataDirPath, "data/bleCommand.json"));
    }
    return bleParamData;
}

QData bleConfig::getSerialData() {
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    if(serialData.empty()){
        serialData.loadFromFile(FileUtils::contactFileName(dataDirPath, "data/serialConfig.json"));
    }
    return serialData;
}

QData bleConfig::getSnAddrData() {
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    if(snAddressData.empty()){
        snAddressData.loadFromFile(FileUtils::contactFileName(dataDirPath, "data/snAddress.json"));
    }
    return snAddressData;
}

void bleConfig::saveSnAddrData(qlibc::QData& data) {
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    snAddressData.setInitData(data);
    snAddressData.saveToFile(FileUtils::contactFileName(dataDirPath, "data/snAddress.json"), true);
}

QData bleConfig::getDeviceListData(){
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    if(deviceListData.empty()){
        deviceListData.loadFromFile(FileUtils::contactFileName(dataDirPath, "data/deviceList.json"));
    }
    return deviceListData;
}

void bleConfig::saveDeviceListData(qlibc::QData& data){
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    deviceListData.setInitData(data);
    deviceListData.saveToFile(FileUtils::contactFileName(dataDirPath, "data/deviceList.json"), true);
}

QData bleConfig::getStatusListData(){
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    if(statusListData.empty()){
        statusListData.loadFromFile(FileUtils::contactFileName(dataDirPath, "data/statusList.json"));
    }
    return statusListData;
}

QData bleConfig::updateStatusListData(qlibc::QData& data){
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    string device_id = data.getString("device_id");
    string state_id = data.getString("state_id");
    string state_value = data.getString("state_value");

    Json::Value device_list = getStatusListData().asValue();
    size_t deviceListSize = device_list.size();
    for(Json::ArrayIndex i = 0; i < deviceListSize; ++i){
        if(device_list[i]["device_id"].asString() == device_id){
            size_t statusListSize = device_list[i]["state_list"].size();
            for(Json::ArrayIndex j = 0; j < statusListSize; ++j){
                if(device_list[i]["state_list"][j]["state_id"].asString() == state_id){
                    device_list[i]["state_list"][j]["state_value"] = state_value;
                    break;
                }
            }
            break;
        }
    }
}

void bleConfig::saveStatusListData(qlibc::QData& data){
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    statusListData.setInitData(data);
    statusListData.saveToFile(FileUtils::contactFileName(dataDirPath, "data/statusList.json"), true);
}

bool bleConfig::serialInit(bleConfig::SerialReceiveFunc receiveFuc) {
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    if(serial == nullptr){
        string serialPort = getSerialData().getString("serial");
        serial.reset(new BLETelinkDongle(serialPort));
        serial->initDongle();
        serial->regRecvDataProc(receiveFuc);
        if(!serial->startDongle()){
            LOG_INFO << "===>startDongle failed, serialPort <" << serialPort << ">.......";
            serial.reset();
            return false;
        }else{
            LOG_INFO << "===>startDongle successfully, serialPort <" << serialPort << ">.......";
            return true;
        }
    }
    return false;
}

shared_ptr<BLETelinkDongle> bleConfig::getSerial() {
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    return serial;
}

void bleConfig::enqueue(std::function<void()> fn) {
    threadPool.enqueue(fn);
}









