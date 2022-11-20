//
// Created by 78472 on 2022/11/17.
//

#ifndef EXHIBITION_SITEMANAGEUTIL_H
#define EXHIBITION_SITEMANAGEUTIL_H

#include <string>
#include <mutex>
#include <unordered_map>
#include "qlibc/QData.h"
#include <ifaddrs.h>
#include <net/if.h>
#include <netdb.h>
#include <thread>

using namespace std;

class SiteTree{
private:
    std::unordered_map<string, Json::Value> siteMap;      //保存在线站点以及离线站点
    std::unordered_map<string, int> sitePingCountMap;     //在线站点的ping计数，<=-3,自动清除
    std::thread* pingThread;
    int pingInterval = 1000;
    std::recursive_mutex siteMutex;

    std::string localIp = "127.0.0.1";
    static SiteTree* Instance;
    SiteTree(){
        initLocalIp();          //获取本机ip
        insertQuerySiteInfo();  //注册查询站点
        //开启ping计数
        pingThread = new thread([this]{
            std::this_thread::sleep_for(std::chrono::seconds(pingInterval));
            pingCountDown();
        });
    }

public:
    //获取单例对象
    static SiteTree* getInstance();

    //站点注册
    void siteRegister(string& siteId, Json::Value& value);

    //站点注销
    void siteUnregister(string& siteId);

    //提取站点消息
    qlibc::QData getSiteInfo(string& siteId);

    //是否存在
    bool isSiteExist(string& siteId);

    //更新站点计数
    void updateSitePingCounter(string& siteId);

    //获取本机ip地址，如果获取网卡地址失败，则为回环口地址
    string getLocalIpAddress();

private:
    //初始化本机IP地址
     void initLocalIp();

    //插入查询站点消息
    void insertQuerySiteInfo();

    //ping计数递减
    void pingCountDown();
};


//站点mdns请求发现返回消息处理
void mdnsResponseHandle(string service_instance_string, string ipString, int sitePort);


#endif //EXHIBITION_SITEMANAGEUTIL_H
