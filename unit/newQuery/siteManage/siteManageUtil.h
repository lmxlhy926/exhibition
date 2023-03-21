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
    std::recursive_mutex siteMutex;                // 保护数据结构并发操作
    std::string localIp = "127.0.0.1";             // 默认IP地址
    std::atomic<bool> initComplete{false};         // 初始化完成

//本机注册站点
    std::unordered_map<string, Json::Value> localSiteMap;           // 本机站点全记录，<siteId, Json::Value>
    std::unordered_map<string, int> localSitePingCountMap;          // 本机在线站点的ping计数，<=-3,自动清除。<siteId, int>
    const int localPingInterval = 10;                               // ping事时间间隔
    std::thread* localPingCoutDownThread;                           // 本机ping计数自减线程

//局域网内其它主机发现、维护
    std::unordered_map<string, Json::Value>  discoveredSiteMap;     // 局域网内发现的其它站点，<主机ip, 对应主机上的所有站点信息>
    std::unordered_map<string, int> discoveredPingCountMap;         // 局域网内发现的节点ping计数，<=-3,自动清除。<主机ip, int>
    int discoverPingInterval = 10;                                  // 主动查询时间间隔
    std::thread* discoveredPingCountDownThread;                     // 发现节点ping计数自减线程
    std::thread* discoverThread;                                    // 查询局域网内其它主机线程

//本机ip确定
    std::unordered_map<string, string> ipMap;      // 本机网卡信息, <ip, name>
    int determineLocalIpInterval = 2;              // 确定ip操作间隔时间
    std::thread* determineLocalIpThread;           // 确认本机IP线程

//启动mdns服务
    std::mutex mdnsMutex;
    std::condition_variable cv;
    bool ready2StartMdnsService{false};
    std::thread* mdnsServiceThread;                 //启动mdns服务线程

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

    //提取本机站点信息条目
    qlibc::QData getLocalSiteInfo(string& siteId);

    //判断站点是否在本机站点
    bool isLocalSiteExist(string& siteId);

    //站点计数复位为1
    void resetLocalSitePingCounter(string& siteId);

    //获取本机ip地址，如果获取网卡地址失败，则为回环口地址
    string getLocalIpAddress();

    /*
     * 处理<查询站点mdns请求报文>返回, 依据返回的ip来判断是否是新的节点
     * 更新局域网主机信息、订阅节点之间通道消息
     */
    void addNewFindSite(string& ip);

    /*
     *  用其它主机的站点上下线消息，更新本地维护的其它主机的站点状态
     */
    void updateFindSite(qlibc::QData& siteInfo);

    /*
     * 获取局域网内所有主机下的指定的站点信息
     * 如果站点为空，则获取局域网内所有站点信息
     */
    qlibc::QData getLocalAreaSite(string& siteId);

    /*
     * 获取局域网内本机外的指定站点信息
     * 如果站点为空，则获取局域网内，本机外的所有站点信息
     */
    qlibc::QData getLocalAreaSiteExceptOwn(string& siteId);

    //返回本地ip集合
    qlibc::QData printIpAddress();

private:
    /*
     *  获取本机网卡信息，记录所有网卡地址，选出合适的地址作为本机IP地址
     *  确定ip地址后，通知mdns服务线程开始mdns服务
     */
     int initLocalIp();

    /*
     * 向本机注册查询站点
     * ！查询站点需要站点自己主动注册
     */
    void insertLocalQuerySiteInfo();

    /*
     * 本机所有站点ping计数递减一次
     * 如果计数 <= -3, 则认为站点离线，移除离线站点，在本机内发送站点离线消息
     */
    void localSitePingCountDown();

    /*
     * 局域网内所有其它主机ping计数递减一次
     * 主机计数 <= -3，认为该主机离线，随后清除离线主机
     */
    void discoveredSitePingCountDown();

    //发送mdns查询报文
    void site_query();

    //开启mdns服务，接受查询报文
    void mdnsServiceStart();

    //更新本机站点信息中的ip地址
    void updateLocalSiteIp();

    //发布列表里站点的上下线消息
    void publishOnOffLineMessage(qlibc::QData& siteList, string onOffLine, bool is2Node);
};

//mdns请求的响应处理
void mdnsResponseHandle(string service_instance_string, string ipString, int sitePort);


#endif //EXHIBITION_SITEMANAGEUTIL_H
