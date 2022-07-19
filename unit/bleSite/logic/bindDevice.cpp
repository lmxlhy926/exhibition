//
// Created by 78472 on 2022/7/19.
//

#include "bindDevice.h"

void bindDevice::operator()() {
    Json::ArrayIndex arraySize = deviceArray_.size();
    if(deviceArray_.type() != Json::arrayValue) return;
    for(Json::ArrayIndex i = 0; i < arraySize; i++){

    }

}

bool bindDevice::addDevice() {

    return false;
}
