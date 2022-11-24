//
// Created by 78472 on 2022/11/17.
//

#ifndef EXHIBITION_SITEMANAGEUTIL_H
#define EXHIBITION_SITEMANAGEUTIL_H

#include <string>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include "qlibc/QData.h"
#include <ifaddrs.h>
#include <net/if.h>
#include <netdb.h>
#include <thread>
#include "mdns/mdnsUtil.h"

using namespace std;

class SiteTree{
private:
    //<siteId, Json::Value>
    std::unordered_map<string, Json::Value> localSiteMap;           //本机站点全记录
    //<siteId, int>
    std::unordered_map<string, int> localSitePingCountMap;          //本机在线站点的ping计数，<=-3,自动清除
    std::thread* localPingThread;                                   //ping线程
    int localPingInterval = 1000;

    //<ip, []>
    std::unordered_map<string, Json::Value>  discoveredSiteMap;     //局域网内发现的其它站点
    //<ip, int>
    std::unordered_map<string, int> discoveredPingCountMap;         //局域网内发现的站点ping计数，<=-3,自动清除
    std::thread* discoverThread;                                    //查询局域网其它节点线程
    int discoverPingInterval = 1000;

    std::recursive_mutex siteMutex;
    std::string localIp = "127.0.0.1";
    static SiteTree* Instance;

    SiteTree(){
        initLocalIp();                  //获取本机ip
        insertLocalQuerySiteInfo();     //注册查询站点
        //开启ping计数
        localPingThread = new thread([this]{
            std::this_thread::sleep_for(std::chrono::seconds(localPingInterval));
            localSitePingCountDown();
        });
        //开启站点查找
        discoverThread = new thread([this]{
            site_query();
            std::this_thread::sleep_for(std::chrono::seconds(discoverPingInterval));
        });
    }

public:
    //获取单例对象
    static SiteTree* getInstance();

    //站点注册
    void siteRegister(string& siteId, Json::Value& value);

    //站点注销
    void siteUnregister(string& siteId);

    //提取本机指定站点消息
    qlibc::QData getLocalSiteInfo(string& siteId);

    //获取本机所有站点信息
    qlibc::QData getAllLocalSiteInfo();

    //判断本地站点是否存在
    bool isLocalSiteExist(string& siteId);

    //更新本机站点计数
    void updateLocalSitePingCounter(string& siteId);

    //获取本机ip地址，如果获取网卡地址失败，则为回环口地址
    string getLocalIpAddress();

    //更新局域网节点、订阅节点的上下线消息
    void addNewFindSite(string& ip);

    //更新发现节点相关站点的上下线消息
    void updateFindSite(qlibc::QData& siteInfo);

    //获取局域网内所有站点信息
    qlibc::QData getAllNetSiteInfo();

private:
    //初始化本机IP地址
     void initLocalIp();

    //插入本机查询站点消息
    void insertLocalQuerySiteInfo();

    //本机站点ping计数递减
    void localSitePingCountDown();

    //发现站点Ping计数递减
    void discoveredSitePingCountDown();

    //查找局域网其它站点
    void site_query();
};


//站点mdns请求发现返回消息处理
void mdnsResponseHandle(string service_instance_string, string ipString, int sitePort);


#endif //EXHIBITION_SITEMANAGEUTIL_H
