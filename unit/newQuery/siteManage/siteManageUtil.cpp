//
// Created by 78472 on 2022/11/17.
//

#include "siteManageUtil.h"
#include <regex>
#include "log/Logging.h"

void mdnsResponseMatch(string name, string ipstr){

    LOG_INFO << "name: " << name;

    smatch sm;
    bool ret = regex_match(ipstr, sm, regex(R"((.*):(.*))"));
    if(ret){
        LOG_INFO << "ipaddress: " << sm[1].str() << ", port: " << sm[2].str();
    }

}

SiteTree* SiteTree::Instance = nullptr;

SiteTree *SiteTree::getInstance() {
   if(Instance == nullptr)
       Instance = new SiteTree();
   return Instance;
}

void SiteTree::siteRegister(string& siteId, Json::Value &value) {
    std::lock_guard<std::recursive_mutex> lg(siteMutex);
    auto pos = siteMap.find(siteId);
    if(pos != siteMap.end())
        siteMap.erase(pos);
    siteMap.insert(std::make_pair(siteId, value));
}

void SiteTree::siteDelete(string& siteId) {
    std::lock_guard<std::recursive_mutex> lg(siteMutex);
    auto pos = siteMap.find(siteId);
    if(pos != siteMap.end())
        siteMap.erase(pos);
}

qlibc::QData SiteTree::siteQuery(string& siteId) {
    std::lock_guard<std::recursive_mutex> lg(siteMutex);
    auto pos = siteMap.find(siteId);
    if(pos != siteMap.end())
       return qlibc::QData(pos->second);
    else
        return qlibc::QData();
}

qlibc::QData SiteTree::extractSiteInfo(string &siteId) {
    std::lock_guard<std::recursive_mutex> lg(siteMutex);
    auto pos = siteMap.find(siteId);
    if(pos != siteMap.end()){
        return qlibc::QData(pos->second);
    }
    return qlibc::QData();
}

bool SiteTree::isSiteExist(string &siteId) {
    std::lock_guard<std::recursive_mutex> lg(siteMutex);
    auto pos = siteMap.find(siteId);
    if(pos != siteMap.end()){
        return true;
    }
    return false;
}
