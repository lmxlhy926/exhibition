//
// Created by 78472 on 2022/7/19.
//

#ifndef EXHIBITION_BINDDEVICE_H
#define EXHIBITION_BINDDEVICE_H

#include "qlibc/QData.h"
#include "snAddressMap.h"

class BindDevice {
private:
    std::unique_ptr<SnAddressMap> snAddrMapPtr;
public:
    explicit BindDevice(){
        snAddrMapPtr.reset(SnAddressMap::getInstance());
    }

    void bind(qlibc::QData& deviceArray);

private:
    bool addDevice(string& deviceSn, Json::ArrayIndex index);
};


#endif //EXHIBITION_BINDDEVICE_H
