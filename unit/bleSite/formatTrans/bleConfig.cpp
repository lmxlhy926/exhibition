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
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    dataDirPath = configPath;
}

string bleConfig::getconfigPath() {
    return dataDirPath;
}

QData bleConfig::getBleParamData() {
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    if(bleParamData.empty()){
        bleParamData.loadFromFile(FileUtils::contactFileName(dataDirPath, "data/bleCommand.json"));
    }
    return bleParamData;
}

QData bleConfig::getSerialData() {
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    if(serialData.empty()){
        serialData.loadFromFile(FileUtils::contactFileName(dataDirPath, "data/serialConfig.json"));
    }
    return serialData;
}

QData bleConfig::getSnAddrData() {
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    if(snAddressData.empty()){
        snAddressData.loadFromFile(FileUtils::contactFileName(dataDirPath, "data/snAddress.json"));
    }
    return snAddressData;
}

void bleConfig::saveSnAddrData(qlibc::QData& data) {
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    snAddressData.setInitData(data);
    snAddressData.saveToFile(FileUtils::contactFileName(dataDirPath, "data/snAddress.json"), true);
}

bool bleConfig::serialInit(bleConfig::SerialReceiveFunc receiveFuc) {
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    if(serial == nullptr){
        string serialPort = getSerialData().getString("serial");
        LOG_INFO << "===>serialPort: " << serialPort;
        serial.reset(new BLETelinkDongle(serialPort));
        serial->initDongle();
        serial->regRecvDataProc(receiveFuc);
        if(!serial->startDongle()){
            LOG_RED << "===>failed in startDongle";
            serial.reset();
            return false;
        }else{
            LOG_INFO << "===>success in startDongle";
            return true;
        }
    }
    return false;
}

shared_ptr<BLETelinkDongle> bleConfig::getSerial() {
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    return serial;
}

void bleConfig::enqueue(std::function<void()> fn) {
    threadPool.enqueue(fn);
}









