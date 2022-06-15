//
// Created by 78472 on 2022/6/7.
//

#include "bleConfigParam.h"
#include "qlibc/FileUtils.h"

bleConfigParam* bleConfigParam::instance = nullptr;

bleConfigParam* bleConfigParam::getInstance() {
    if(instance == nullptr)
        instance = new bleConfigParam;
    return instance;
}

void bleConfigParam::setConfigPath(const string& configPath) {
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    dataDirPath = configPath;
}

string bleConfigParam::getconfigPath() {
    return dataDirPath;
}

QData bleConfigParam::getBleParamData() {
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    if(bleParamData.empty()){
        bleParamData.loadFromFile(FileUtils::contactFileName(dataDirPath, "data/bleCommand.json"));
    }
    return bleParamData;
}


