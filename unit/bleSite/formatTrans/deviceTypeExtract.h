//
// Created by 78472 on 2022/9/9.
//

#ifndef EXHIBITION_DEVICETYPEEXTRACT_H
#define EXHIBITION_DEVICETYPEEXTRACT_H

#include <string>

using namespace std;

class deviceTypeExtract {
private:
    std::string deviceUUID;

public:
    explicit deviceTypeExtract(string& uuid) : deviceUUID(uuid){}

    string getDeviceType();
};


#endif //EXHIBITION_DEVICETYPEEXTRACT_H
