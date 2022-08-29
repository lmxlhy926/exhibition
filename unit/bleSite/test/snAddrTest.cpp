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
#include "qlibc/jsoncpp/json.h"


int main(int argc, char* argv[]){
    bleConfig* configPathPtr = bleConfig::getInstance();
    configPathPtr->setConfigPath(string(argv[1]));

    SnAddressMap *samptr = SnAddressMap::getInstance();
    string deviceSn = "0001";
    LOG_RED << samptr->getNodeAssignAddr(deviceSn).toJsonString(true);

    return 0;
}
