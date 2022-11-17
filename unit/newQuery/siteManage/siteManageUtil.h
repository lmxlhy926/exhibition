//
// Created by 78472 on 2022/11/17.
//

#ifndef EXHIBITION_SITEMANAGEUTIL_H
#define EXHIBITION_SITEMANAGEUTIL_H

#include <string>
#include <mutex>
#include <unordered_map>
#include "qlibc/QData.h"

using namespace std;

void mdnsResponseMatch(string name, string ipstr);

class SiteTree{
private:
    std::unordered_map<string, Json::Value> siteMap;
    std::recursive_mutex siteMutex;
    static SiteTree* Instance;
    SiteTree(){
        siteMap.insert(std::make_pair("site-query", Json::Value()));
    }

public:
    //获取单例对象
    static SiteTree* getInstance();

    //站点注册
    void siteRegister(string& siteId, Json::Value& value);

    //站点注销
    void siteDelete(string& siteId);

    //站点查询
    qlibc::QData siteQuery(string& siteId);

    //提取站点消息
    qlibc::QData extractSiteInfo(string& siteId);

    //是否存在
    bool isSiteExist(string& siteId);
};




#endif //EXHIBITION_SITEMANAGEUTIL_H
