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
    {
        std::lock_guard<std::recursive_mutex> lg(siteMutex);
        //更新在线站点信息
        auto pos = localSiteMap.find(siteId);
        if(pos != localSiteMap.end())
            localSiteMap.erase(pos);
        localSiteMap.insert(std::make_pair(siteId, value));

        //更新在线站点计数
        auto pos1 = localSitePingCountMap.find(siteId);
        if(pos1 != localSitePingCountMap.end())
            localSitePingCountMap.erase(pos1);
        localSitePingCountMap.insert(std::make_pair(siteId, 1));
    }

    //发布站点上线消息
    qlibc::QData siteOnOffData;
    siteOnOffData.setString("message_id", Site_OnOffLine_MessageID);
    siteOnOffData.setValue("content", value);
    ServiceSiteManager::getInstance()->publishMessage(Site_OnOffLine_MessageID, siteOnOffData.toJsonString());
}

//站点注销：移除离线站点、发布站点离线消息
void SiteTree::siteUnregister(string& siteId) {
    bool flag = false;
    Json::Value value;
    {
        std::lock_guard<std::recursive_mutex> lg(siteMutex);
        auto pos = localSiteMap.find(siteId);
        if(pos != localSiteMap.end()){
            pos->second["site_status"] = "offline";
            flag = true;
            value = pos->second;
            localSiteMap.erase(siteId);
            localSitePingCountMap.erase(siteId);
        }
    }

    if(flag){
        //发布站点下线消息
        qlibc::QData siteOnOffData;
        siteOnOffData.setString("message_id", Site_OnOffLine_MessageID);
        siteOnOffData.setValue("content", value);
        ServiceSiteManager::getInstance()->publishMessage(Site_OnOffLine_MessageID, siteOnOffData.toJsonString());
    }
}

//提取在线站点注册消息
qlibc::QData SiteTree::getSiteInfo(string &siteId) {
    std::lock_guard<std::recursive_mutex> lg(siteMutex);
    auto pos = localSiteMap.find(siteId);
    if(pos != localSiteMap.end()){
        return qlibc::QData(pos->second);
    }
    return qlibc::QData();
}


//判断本地站点是否为在线站点
bool SiteTree::isLocalSiteExist(string &siteId) {
    std::lock_guard<std::recursive_mutex> lg(siteMutex);
    auto pos = localSiteMap.find(siteId);
    if(pos != localSiteMap.end()){
        return true;
    }
    return false;
}

//更新本机站点计数
void SiteTree::updateLocalSitePingCounter(string& siteId){
    std::lock_guard<std::recursive_mutex> lg(siteMutex);
    auto pos = localSitePingCountMap.find(siteId);
    if(pos != localSitePingCountMap.end()){
        pos->second = 1;
    }
}

string SiteTree::getLocalIpAddress() {
   return localIp;
}


void SiteTree::updateFindSite(string& ip) {
    {
        std::lock_guard<std::recursive_mutex> lg(siteMutex);
        auto pos = discoveredSite.find(ip);
        if(pos == discoveredSite.end()){   //插入新发现的节点
            if(ip != localIp){
                discoveredSite.insert(ip);
            }
        }
    }

    //不是本机站点，则订阅该站点的上下线消息
    if(ip != localIp){
        std::vector<string> messageIdList;
        messageIdList.push_back(Site_OnOffLine_MessageID);
        ServiceSiteManager::subscribeMessage(ip, 9000, messageIdList);
    }
}

qlibc::QData SiteTree::getAllLocalAreaSite() {
    return qlibc::QData();
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

void SiteTree::insertLocalQuerySiteInfo() {
    std::lock_guard<std::recursive_mutex> lg(siteMutex);
    Json::Value querySiteInfo;
    querySiteInfo["ip"] = localIp;
    querySiteInfo["port"] = 9000;
    querySiteInfo["site_id"] = "site_query";
    querySiteInfo["site_status"] = "online";
    querySiteInfo["summary"] = "服务站点查询";
    localSiteMap.insert(std::make_pair("site-query", querySiteInfo));
    localSitePingCountMap.insert(std::make_pair("site-query", 1));
}

//站点计数-1，移除离线站点计数、发布站点离线消息
void SiteTree::localSitePingCountDown(){
    qlibc::QData offlineList;
    {
        std::lock_guard<std::recursive_mutex> lg(siteMutex);
        for(auto& elem : localSitePingCountMap){
            if(elem.first != "site_query"){
                elem.second += -1;
            }
        }

        for(auto pos = localSitePingCountMap.begin(); pos != localSitePingCountMap.end();){
            if(pos->second <= -3){
                auto elemPos = localSiteMap.find(pos->first);
                if(elemPos != localSiteMap.end()){
                    elemPos->second["site_status"] = "offline";
                    offlineList.append(elemPos->second);
                    localSiteMap.erase(elemPos);
                }
                //移除离线站点的计数
                pos = localSitePingCountMap.erase(pos);
            }
        }
    }

    //发布离线站点消息
    Json::ArrayIndex size = offlineList.size();
    for(int i = 0; i < size; ++i){
        qlibc::QData ithData = offlineList.getArrayElement(i);
        //发布站点下线消息
        qlibc::QData siteOnOffData;
        siteOnOffData.setString("message_id", Site_OnOffLine_MessageID);
        siteOnOffData.setValue("content", ithData.asValue());
        ServiceSiteManager::getInstance()->publishMessage(Site_OnOffLine_MessageID, siteOnOffData.toJsonString());
    }
}

void SiteTree::site_query() {
    string querySiteName = "_edgeai.site-query._tcp.local.";
    mdns_query_t query[1];
    query[0].name = querySiteName.c_str();
    query[0].length = strlen(query[0].name);
    query[0].type = MDNS_RECORDTYPE_PTR;
    send_mdns_query(query, 1);
}


//处理mdns请求返回，发布站点信息
void mdnsResponseHandle(string service_instance_string, string ipString, int sitePort){
    smatch sm;
    bool matchRet = regex_match(service_instance_string, sm, regex("(.*)[.](.*)[.](.*)[.](.*)[.](.*)[.]"));
    if(matchRet){
        string site_id = sm.str(3);
        if(regex_match(ipString, sm, regex(R"((.*):(.*))"))){
            string ip = sm.str(1);
            string siteName = sm.str(3);
            if(siteName == "site_query"){
                //更新局域网发现的站点
                SiteTree::getInstance()->updateFindSite(ipString);
            }

            //构造发布消息
            qlibc::QData content, publishData;
            content.setString("site_id", site_id);
            content.setString("ip", ip);
            content.setInt("port", sitePort);
            publishData.setString("message_id", "site_query_result");
            publishData.putData("content", content);
            ServiceSiteManager::getInstance()->publishMessage(Site_Requery_Result_MessageID, publishData.toJsonString());
        }
    }
}
