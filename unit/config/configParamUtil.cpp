//
// Created by 78472 on 2022/5/11.
//

#include "configParamUtil.h"

#include <utility>

configParamUtil* configParamUtil::instance = nullptr;

configParamUtil::configParamUtil(){}

configParamUtil *configParamUtil::getInstance() {
    if(instance == nullptr)
        instance = new configParamUtil;
    return instance;
}

void configParamUtil::setConfigPath(const string& configPath) {
    dataDirPath = configPath;
}

string configParamUtil::getconfigPath() {
    return dataDirPath;
}


