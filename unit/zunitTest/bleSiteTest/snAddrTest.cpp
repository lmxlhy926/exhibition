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
    bleConfig* configPathPtr = bleConfig::getInstance();
    configPathPtr->setConfigPath(string(argv[1]));

    GroupAddressMap *gamptr = GroupAddressMap::getInstance();

    //创建分组
    string groupName1 = "group1";
    string groupName2 = "group2";
    string groupName3 = "group4";
//    LOG_INFO << gamptr->createGroup(groupName1);
//    LOG_INFO << gamptr->createGroup(groupName2);
//    LOG_INFO << gamptr->createGroup(groupName3);

    //删除分组
    string groupID = "02C0";
//    gamptr->deleGroup(groupID);

    //设备加入分组
    string deviceSn1 = "sn1";
    string deviceSn2 = "sn2";
//    gamptr->addDevice2Group(groupID, deviceSn1);
//    gamptr->addDevice2Group(groupID, deviceSn2);

    //设备从分组剔除
//    gamptr->removeDeviceFromGroup(groupID, deviceSn2);

//    LOG_INFO << gamptr->getGroupList().toJsonString(true);

    LOG_INFO << gamptr->groupName2GroupAddressId(groupName3);
    LOG_INFO << gamptr->groupAddressId2GroupName(groupID);

    return 0;
}
