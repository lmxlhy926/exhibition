//
// Created by 78472 on 2022/11/17.
//

#ifndef EXHIBITION_SITEMANAGEUTIL_H
#define EXHIBITION_SITEMANAGEUTIL_H

#include <string>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <unordered_set>
#include <atomic>
#include "qlibc/QData.h"
#include <ifaddrs.h>
#include <net/if.h>
#include <netdb.h>
#include <thread>
#include "mdns/mdnsUtil.h"
#include "log/Logging.h"

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
//本机注册站点
    std::unordered_map<string, Json::Value> localSiteMap;           // 本机站点全记录，<siteId, Json::Value>
    std::unordered_map<string, int> localSitePingCountMap;          // 本机在线站点的ping计数，<=-3,自动清除。<siteId, int>
    const int localPingInterval = 10;                               // ping事时间间隔
    std::thread* localPingCoutDownThread;                           // 本机ping自减线程

//局域网内其它主机
    std::unordered_map<string, Json::Value>  discoveredSiteMap;     // 局域网内发现的其它站点，<主机ip, 对应主机上的所有站点信息>
    std::unordered_map<string, int> discoveredPingCountMap;         // 局域网内发现的节点ping计数，<=-3,自动清除。<主机ip, int>
    int discoverPingInterval = 5;                                   // 主动查询时间间隔
    std::thread* discoveredPingCountDownThread;                     // 发现节点ping线程
    std::thread* discoverThread;                                    // 查询局域网内其它主机线程

    std::recursive_mutex siteMutex;                // 保护数据结构并发操作
    std::string localIp = "127.0.0.1";             // 默认IP地址
    std::atomic<bool> initComplete{false};       // 初始化完成

//本机ip确定
    std::unordered_map<string, string> ipMap;      // 本机网卡信息, <ip, name>
    const int determineLocalIpInterval = 2;        // 确定ip操作间隔时间
    std::thread* determineLocalIpThread;           // 确认本机IP线程

//启动mdns服务
    std::mutex mdnsMutex;
    std::condition_variable cv;
    bool ready2StartMdnsService{false};
    std::thread* mdnsServiceThread;                //启动mdns服务线程

    static SiteTree* Instance;

public:
    //获取单例对象
    static SiteTree* getInstance();

    //初始化操作，确定ip，插入本地查询站点，开启各个运行线程
    void init();

    //站点注册
    void siteRegister(string& siteId, Json::Value& value);

    //站点注销
    void siteUnregister(string& siteId);

    //提取本机站点消息
    qlibc::QData getLocalSiteInfo(string& siteId);

    //判断本地站点是否存在
    bool isLocalSiteExist(string& siteId);

    //站点计数归一
    void resetLocalSitePingCounter(string& siteId);

    //获取本机ip地址，如果获取网卡地址失败，则为回环口地址
    string getLocalIpAddress();

    /*
     * 发送查询请求，依据返回的ip来判断是否是新的节点
     * 更新局域网节点信息、订阅节点之间通道消息
     */
    void addNewFindSite(string& ip);

    //用节点之间的消息，更新本机维护的节点下的站点状态
    // 根据其它主机发送的
    void updateFindSite(qlibc::QData& siteInfo);

    //获取局域网内指定的站点信息（一个或多个）
    qlibc::QData getLocalAreaSite(string& siteId);

    //打印资源情况
    qlibc::QData printResource();

private:
    // 获取本地网卡信息，确定本机ip地址有效，通知mdns服务开启
     int initLocalIp();

    //向本机注册查询站点
    void insertLocalQuerySiteInfo();

    //本机站点ping计数递减一次，移除离线站点
    void localSitePingCountDown();

    //局域网内其他主机ping计数递减一次，移除离线主机
    void discoveredSitePingCountDown();

    //发送mdns查询报文
    void site_query();

    //开启mdns服务
    void mdnsServiceStart();

    //更新本机站点信息中的ip地址
    void updateLocalSiteIp();

    //发布列表里站点的上下线消息
    void publishOnOffLineMessage(qlibc::QData& siteList, string onOffLine, bool is2Node);
};


//站点mdns请求发现返回消息处理
void mdnsResponseHandle(string service_instance_string, string ipString, int sitePort);


#endif //EXHIBITION_SITEMANAGEUTIL_H
