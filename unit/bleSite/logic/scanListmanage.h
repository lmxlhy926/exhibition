//
// Created by 78472 on 2022/9/24.
//

#ifndef EXHIBITION_SCANLISTMANAGE_H
#define EXHIBITION_SCANLISTMANAGE_H

#include <vector>
#include <map>
#include <mutex>
#include "qlibc/QData.h"

class ScanListmanage {
private:
    static ScanListmanage* instance;
    std::map<string, Json::Value> list;
    std::recursive_mutex rMutex_;
    ScanListmanage();
public:
    static ScanListmanage* getInstance();

    void appendDeviceItem(string deviceSn, Json::Value property);

    void deleteDeviceItem(string& deviceSn);

    std::map<string, Json::Value> getScanListMap();

    void loadData();

    void saveData();
};


#endif //EXHIBITION_SCANLISTMANAGE_H
