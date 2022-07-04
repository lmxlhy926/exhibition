//
// Created by 78472 on 2022/6/7.
//

#include "bleConfig.h"
#include "qlibc/FileUtils.h"
#include <iostream>

bleConfig* bleConfig::instance = nullptr;

bleConfig* bleConfig::getInstance() {
    if(instance == nullptr)
        instance = new bleConfig;
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

bool bleConfig::serialInit(bleConfig::SerialReceiveFunc receiveFuc) {
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    if(serial == nullptr){
        string serialPort = getSerialData().getString("serial");
        std::cout << "===>serialPort: " << serialPort << std::endl;
        serial.reset(new BLETelinkDongle(serialPort));
        serial->initDongle();
        serial->regRecvDataProc(receiveFuc);
        if(!serial->startDongle()){
            std::cout << "===>failed in startDongle" << std::endl;
            serial.reset();
            return false;
        }else{
            std::cout << "===>success in startDongle" << std::endl;
            return true;
        }
    }
    return false;
}

shared_ptr<BLETelinkDongle> bleConfig::getSerial() {
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    return serial;
}







