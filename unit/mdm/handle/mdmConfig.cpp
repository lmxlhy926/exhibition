//
// Created by 78472 on 2022/10/22.
//

#include "mdmConfig.h"
#include "qlibc/FileUtils.h"

mdmConfig* mdmConfig::instance = nullptr;

mdmConfig *mdmConfig::getInstance() {
   if(instance == nullptr){
       instance = new mdmConfig();
   }
   return instance;
}

void mdmConfig::setConfigPath(const string &configPath) {
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    dataDirPath = configPath;
}

string mdmConfig::getconfigPath() {
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    return dataDirPath;
}

string mdmConfig::getCaPath(){
    return FileUtils::contactFileName(dataDirPath, "data/ca-bundle.crt");
}

QData mdmConfig::getMqttConfigData() {
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    if(mqttConfigData.empty()){
        mqttConfigData.loadFromFile(FileUtils::contactFileName(dataDirPath, "data/mqttConfig.json"));
    }
    return mqttConfigData;
}

QData mdmConfig::getBaseInfoData() {
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    if(baseInfoData.empty()){
        baseInfoData.loadFromFile(FileUtils::contactFileName(dataDirPath, "data/baseInfo.json"));
    }
    return baseInfoData;
}

QData mdmConfig::getAuthInfoData() {
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    if(authInfoData.empty()){
        authInfoData.loadFromFile(FileUtils::contactFileName(dataDirPath, "data/authInfo.json"));
    }
    return authInfoData;
}

void mdmConfig::saveAuthInfoData(qlibc::QData& data) {
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    authInfoData.setInitData(data);
    authInfoData.saveToFile(FileUtils::contactFileName(dataDirPath, "data/authInfo.json"), true);

}
