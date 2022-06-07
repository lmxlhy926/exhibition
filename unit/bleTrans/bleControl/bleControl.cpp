//
// Created by 78472 on 2022/6/7.
//

#include "bleControl.h"
#include "../bleConfigParam.h"
#include <iostream>

void control(qlibc::QData& data){
    std::cout << "===>data: " << data.toJsonString() << std::endl;
    string command = data.getData("request").getString("command");

    bleConfigParam* configPathPtr = bleConfigParam::getInstance();
    qlibc::QData bleConfigData = configPathPtr->getBleParamData();
    std::cout << "bleConfigData: " << bleConfigData.toJsonString() << std::endl;

    std::vector<string> orderVec;
    qlibc::QData order = bleConfigData.getData("commonBase").getData("ORDER");
    for(int i = 0; i < order.size(); i++){
        orderVec.push_back(order.getArrayElement(i).asValue().asString());
    }

    string binaryCommandString;
    for(auto &elem : orderVec){
        binaryCommandString += bleConfigData.getData("commonBase").getData("param").getString(elem);
    }

    std::cout << "==>binaryCommandString: " << binaryCommandString << std::endl;
}

