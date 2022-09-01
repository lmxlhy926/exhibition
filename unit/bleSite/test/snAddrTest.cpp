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

#include "logic/groupAddressMap.h"

void snAddrTest(char* argv[]){
    bleConfig* configPathPtr = bleConfig::getInstance();
    configPathPtr->setConfigPath(string(argv[1]));

    SnAddressMap *samptr = SnAddressMap::getInstance();
    string deviceSn = "0001";
    LOG_RED << samptr->getNodeAssignAddr(deviceSn).toJsonString(true);
}


int main(int argc, char* argv[]){
//    bleConfig* configPathPtr = bleConfig::getInstance();
//    configPathPtr->setConfigPath(string(argv[1]));
//
//    GroupAddressMap *gamptr = GroupAddressMap::getInstance();
//    string groupName = "group6";
//    string deviceSn = "mac3";
//    gamptr->addDevice2Group(groupName, deviceSn);

    stringstream ss;
    int ctlTemperature = 0;
    ss << std::setfill('0') << std::hex << std::uppercase << std::setw(4) << ctlTemperature;
    std::cout << ss.str() << std::endl;


    return 0;
}
