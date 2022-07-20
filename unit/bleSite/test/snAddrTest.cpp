//
// Created by 78472 on 2022/7/20.
//

#include <iostream>
#include "logic/snAddressMap.h"
#include "log/Logging.h"
#include "qlibc/QData.h"
#include "formatTrans/bleConfig.h"


int main(int argc, char* argv[]){
    bleConfig* configPathPtr = bleConfig::getInstance();
    configPathPtr->setConfigPath(string(argv[1]));

    SnAddressMap sam;
    LOG_RED << sam.getNodeAssignAddr("abcdef").toJsonString(true);
    return 0;
}
