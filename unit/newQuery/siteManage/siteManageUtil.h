//
// Created by 78472 on 2022/11/17.
//

#ifndef EXHIBITION_SITEMANAGEUTIL_H
#define EXHIBITION_SITEMANAGEUTIL_H

#include <string>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <atomic>
#include "qlibc/QData.h"
#include <ifaddrs.h>
#include <net/if.h>
#include <netdb.h>
#include <thread>
#include "mdns/mdnsUtil.h"

using namespace std;

/*
 *  维护本机站点
 *  维护网络中发现的其它节点
 *  供应用查询
 *  多线程调用，需要为线程安全函数
 *  锁保护中不含耗时操作
 *
 */
class SiteTree{
private:
//本机站点维护
    //<siteId, Json::Value>
    std::unordered_map<string, Json::Value> localSiteMap;           // 本机站点全记录
    //<siteId, int>
    std::unordered_map<string, int> localSitePingCountMap;          // 本机在线站点的ping计数，<=-3,自动清除
    std::thread* localPingThread;                                   // ping线程
    int localPingInterval = 1000;

//被发现的节点的维护
    //<节点ip, [所有节点包含的站点信息]>
    std::unordered_map<string, Json::Value>  discoveredSiteMap;     // 局域网内发现的其它站点
    //<ip, int>
    std::unordered_map<string, int> discoveredPingCountMap;         // 局域网内发现的节点ping计数，<=-3,自动清除
    std::thread* discoverThread;                                    // 查询局域网其它节点线程
    std::thread* discoveredPingThread;                              // 发现节点ping线程
    int discoverPingInterval = 1000;

    std::recursive_mutex siteMutex;                // 保护数据结构并发操作
    std::atomic<bool> ipConfirm{false};         // 本机有效IP地址确认标志
    std::atomic<bool> initComplete{false};      //初始化完成
    std::string localIp = "127.0.0.1";            // 默认IP地址
    std::unordered_set<string> ipSet;             // 本机可用的ip地址集

    static SiteTree* Instance;

public:
    //获取单例对象
    static SiteTree* getInstance();

    void init(){
        initLocalIp();     //获取本机所有IP
        site_query();      //通过返回确定本机有效IP
        insertLocalQuerySiteInfo(); //确定有效IP后，注册查询站点

        //本机站点ping计数
        localPingThread = new thread([this]{
            while(true){
                std::this_thread::sleep_for(std::chrono::seconds(localPingInterval));
                localSitePingCountDown();
            }
        });

        //开启站点查找
        discoverThread = new thread([this]{
            while(true){
                std::this_thread::sleep_for(std::chrono::seconds(discoverPingInterval));
                site_query();
            }
        });

        //发现节点ping计数
        discoveredPingThread = new thread([this]{
            while(true){
                std::this_thread::sleep_for(std::chrono::seconds(localPingInterval));
                discoveredSitePingCountDown();
            }
        });

        initComplete.store(true);
    }

    //站点注册
    void siteRegister(string& siteId, Json::Value& value);

    //站点注销
    void siteUnregister(string& siteId);

    //提取本机站点消息
    qlibc::QData getLocalSiteInfo(string& siteId);

    //判断本地站点是否存在
    bool isLocalSiteExist(string& siteId);

    //更新本机站点计数
    void updateLocalSitePingCounter(string& siteId);

    //确定本机使用的IP地址
    void confirmIp(string& ip);

    //获取本机ip地址，如果获取网卡地址失败，则为回环口地址
    string getLocalIpAddress();

    //更新局域网节点信息、订阅节点之间通道消息
    void addNewFindSite(string& ip);

    //用节点之间消息，更新本机维护的发现节点下挂的站点信息
    void updateFindSite(qlibc::QData& siteInfo);

    //获取局域网内指定的站点信息
    qlibc::QData getLocalAreaSite(string& siteId);

private:
    //初始化本机IP地址
     void initLocalIp();

    //插入本机查询站点消息
    void insertLocalQuerySiteInfo();

    //本机站点ping计数递减
    void localSitePingCountDown();

    //发现站点Ping计数递减, 检测维护的节点是否下线
    void discoveredSitePingCountDown();

    //查找局域网其它站点
    void site_query();

    //发布列表里站点的上下线消息
    void publishOnOffLineMessage(qlibc::QData& siteList, string onOffLine, bool is2Node);
};


//站点mdns请求发现返回消息处理
void mdnsResponseHandle(string service_instance_string, string ipString, int sitePort);


#endif //EXHIBITION_SITEMANAGEUTIL_H
