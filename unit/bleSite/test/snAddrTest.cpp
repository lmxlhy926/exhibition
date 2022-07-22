//
// Created by 78472 on 2022/7/20.
//

#include <iostream>
#include "logic/snAddressMap.h"
#include "log/Logging.h"
#include "qlibc/QData.h"
#include "formatTrans/bleConfig.h"
#include <vector>
#include <algorithm>


int main(int argc, char* argv[]){
    bleConfig* configPathPtr = bleConfig::getInstance();
    configPathPtr->setConfigPath(string(argv[1]));

    SnAddressMap sam;
    for(int i = 0; i < 11; ++i){
        string deviceSn = string("device").append(std::to_string(i));
        LOG_RED << sam.getNodeAssignAddr(deviceSn).toJsonString(true);
    }
    LOG_HLIGHT << sam.getDeviceList().toJsonString();

    return 0;
}
