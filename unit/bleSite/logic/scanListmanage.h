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

    //增加扫描列表条目
    void appendDeviceItem(string deviceSn, Json::Value property);

    //删除扫描列表条目
    void deleteDeviceItem(string& deviceSn);

    std::map<string, Json::Value> getScanListMap();

    //加载扫描列表数据
    void loadData();

    //存储扫描列表数据
    void saveData();
};


#endif //EXHIBITION_SCANLISTMANAGE_H
