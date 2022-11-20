//
// Created by 78472 on 2022/11/17.
//

#include "siteManageUtil.h"
#include <regex>
#include "log/Logging.h"
#include "mdns/mdnsUtil.h"
#include "siteService/nlohmann/json.hpp"
#include "siteService/service_site_manager.h"
#include "../param.h"

using namespace servicesite;

SiteTree* SiteTree::Instance = nullptr;

SiteTree *SiteTree::getInstance() {
   if(Instance == nullptr)
       Instance = new SiteTree();
   return Instance;
}

//站点注册：更新siteMap, sitePingCountMap
void SiteTree::siteRegister(string& siteId, Json::Value &value) {
    std::lock_guard<std::recursive_mutex> lg(siteMutex);
    //插入sieMap
    auto pos = siteMap.find(siteId);
    if(pos != siteMap.end())
        siteMap.erase(pos);
    siteMap.insert(std::make_pair(siteId, value));

    //出入sitePingCountMap
    auto pos1 = sitePingCountMap.find(siteId);
    if(pos1 != sitePingCountMap.end())
        sitePingCountMap.erase(pos1);
    sitePingCountMap.insert(std::make_pair(siteId, 1));

    //发布站点上线消息
    qlibc::QData siteOnOffData;
    siteOnOffData.setString("message_id", Site_OnOffLine_MessageID);
    siteOnOffData.setValue("content", value);
    ServiceSiteManager::getInstance()->publishMessage(Site_OnOffLine_MessageID, siteOnOffData.toJsonString());
}

//站点注销：站点变为离线，移除站点计数
void SiteTree::siteUnregister(string& siteId) {
    std::lock_guard<std::recursive_mutex> lg(siteMutex);
    auto pos = siteMap.find(siteId);
    if(pos != siteMap.end()){
        pos->second["site_status"] = "offline";
        sitePingCountMap.erase(siteId);

        //发布站点下线消息
        qlibc::QData siteOnOffData;
        siteOnOffData.setString("message_id", Site_OnOffLine_MessageID);
        siteOnOffData.setValue("content", pos->second);
        ServiceSiteManager::getInstance()->publishMessage(Site_OnOffLine_MessageID, siteOnOffData.toJsonString());
    }
}

qlibc::QData SiteTree::getSiteInfo(string &siteId) {
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
        if(pos->second["site_status"] == "online")
            return true;
        else
            return false;
    }
    return false;
}

void SiteTree::updateSitePingCounter(string& siteId){
    std::lock_guard<std::recursive_mutex> lg(siteMutex);
    auto pos = sitePingCountMap.find(siteId);
    if(pos != sitePingCountMap.end()){
        pos->second = 1;
    }
}

string SiteTree::getLocalIpAddress() {
   return localIp;
}


void SiteTree::initLocalIp() {
    struct ifaddrs* ifaddr = nullptr;
    struct ifaddrs* ifa = nullptr;

    //获取本机所有网卡的网卡地址
    if (getifaddrs(&ifaddr) < 0){
        printf("Unable to get interface addresses, return loopback address...\n");
        return;
    }

    char buffer[128];
    for (ifa = ifaddr; ifa; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr)  //网卡地址存在
            continue;
        if (!(ifa->ifa_flags & IFF_UP))  //网口有效
            continue;
        if ((ifa->ifa_flags & IFF_LOOPBACK) || (ifa->ifa_flags & IFF_POINTOPOINT))  //不是回环口地址、不是点对点网络
            continue;

        if (ifa->ifa_addr->sa_family == AF_INET) {
            struct sockaddr_in* saddr = (struct sockaddr_in*)ifa->ifa_addr;
            if (saddr->sin_addr.s_addr != htonl(INADDR_LOOPBACK)) {  //不是回环口地址
                mdns_string_t addr = ipv4_address_to_string(buffer, sizeof(buffer),
                                                            saddr,sizeof(struct sockaddr_in));
                localIp = string(addr.str, addr.length);
                break;
            }
        }
    }
    freeifaddrs(ifaddr);
}

void SiteTree::insertQuerySiteInfo() {
    std::lock_guard<std::recursive_mutex> lg(siteMutex);
    Json::Value querySiteInfo;
    querySiteInfo["ip"] = localIp;
    querySiteInfo["port"] = 9000;
    querySiteInfo["site_id"] = "site_query";
    querySiteInfo["site_status"] = "online";
    querySiteInfo["summary"] = "服务站点查询";
    siteMap.insert(std::make_pair("site-query", querySiteInfo));
    sitePingCountMap.insert(std::make_pair("site-query", 1));
}

//站点计数-1，移除离线站点计数、发布站点离线消息
void SiteTree::pingCountDown(){
    std::lock_guard<std::recursive_mutex> lg(siteMutex);
    for(auto& elem : sitePingCountMap){
        if(elem.first != "site_query"){
            elem.second += -1;
        }
    }

    for(auto pos = sitePingCountMap.begin(); pos != sitePingCountMap.end();){
        if(pos->second <= -3){
            auto elemPos = siteMap.find(pos->first);
            if(elemPos != siteMap.end()){
                elemPos->second["site_status"] = "offline";
                //发布站点下线消息
                qlibc::QData siteOnOffData;
                siteOnOffData.setString("message_id", Site_OnOffLine_MessageID);
                siteOnOffData.setValue("content", elemPos->second);
                ServiceSiteManager::getInstance()->publishMessage(Site_OnOffLine_MessageID, siteOnOffData.toJsonString());
            }

            //移除离线站点的计数
            pos = sitePingCountMap.erase(pos);
        }
    }
}

//处理mdns请求返回，发布站点信息
void mdnsResponseHandle(string service_instance_string, string ipString, int sitePort){
    smatch sm;
    bool matchRet = regex_match(service_instance_string, sm, regex("(.*)[.](.*)[.](.*)[.](.*)[.](.*)[.]"));
    if(matchRet){
        string site_id = sm.str(3);
        if(regex_match(ipString, sm, regex(R"((.*):(.*))"))){
            string ip = sm.str(1);

            //构造发布消息
            qlibc::QData content, publishData;
            content.setString("site_id", site_id);
            content.setString("ip", ip);
            content.setInt("port", sitePort);
            publishData.setString("message_id", "site_query_result");
            publishData.putData("content", content);
            LOG_INFO << publishData.toJsonString();
            ServiceSiteManager::getInstance()->publishMessage(Site_Requery_Result_MessageID, publishData.toJsonString());
        }
    }
}
