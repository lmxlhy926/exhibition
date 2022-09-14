//
// Created by 78472 on 2022/9/9.
//

#ifndef EXHIBITION_DEVICETYPEEXTRACT_H
#define EXHIBITION_DEVICETYPEEXTRACT_H

#include <string>
#include "qlibc/QData.h"
#include "bleConfig.h"

using namespace std;

class deviceTypeExtract {
private:
    std::string deviceUUID;
    qlibc::QData deviceTypeData;

public:
    explicit deviceTypeExtract(string& uuid) : deviceUUID(uuid){
        initDeviceTypeData();
    }

    string getDeviceModel();

    string getDeviceModelCode();

    string getDeviceType();

    string getDeviceTypeCode();

private:
    void initDeviceTypeData();
};


#endif //EXHIBITION_DEVICETYPEEXTRACT_H
