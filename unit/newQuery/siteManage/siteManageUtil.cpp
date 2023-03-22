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
#include "common/httpUtil.h"

using namespace servicesite;

SiteTree* SiteTree::Instance = nullptr;

SiteTree *SiteTree::getInstance() {
   if(Instance == nullptr)
       Instance = new SiteTree();
   return Instance;
}

void SiteTree::init(){
    initLocalIp();
    insertLocalQuerySiteInfo(); //确定有效IP后，自动注册查询站点

    //开启主机IP确认线程
    determineLocalIpThread = new thread([this]{
        while(true){
            if(determineLocalIpInterval < 30){
                determineLocalIpInterval *= 2;
            }
            std::this_thread::sleep_for(std::chrono::seconds(determineLocalIpInterval));
            initLocalIp();
        }
    });

    //开启主机发现线程，定时发送查询请求
    discoverThread = new thread([this]{
        while(true){
            std::this_thread::sleep_for(std::chrono::seconds(discoverPingInterval));
            site_query();
        }
    });

    //局域网内其它主机ping计数递减线程
    discoveredPingCountDownThread = new thread([this]{
        while(true){
            std::this_thread::sleep_for(std::chrono::seconds(discoverPingInterval));
            discoveredSitePingCountDown();
        }
    });

    //本机站点ping计数递减线程
    localPingCoutDownThread = new thread([this]{
        while(true){
            std::this_thread::sleep_for(std::chrono::seconds(localPingInterval));
            localSitePingCountDown();
        }
    });

    //开启mdns服务线程
    mdnsServiceThread = new thread([this]{
        mdnsServiceStart();
    });

    initComplete.store(true);
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

    //在本机发布站点上线消息
    qlibc::QData siteOnOffData;
    siteOnOffData.setString("message_id", Site_OnOffLine_MessageID);
    siteOnOffData.setValue("content", value);
    ServiceSiteManager::getInstance()->publishMessage(Site_OnOffLine_MessageID, siteOnOffData.toJsonString());
    LOG_INFO << "Publish onoffline: " << siteOnOffData.toJsonString();

    //向其它主机发布站点上线消息
    siteOnOffData.setString("message_id", Site_OnOff_Node2Node_MessageID);
    ServiceSiteManager::getInstance()->publishMessage(Site_OnOff_Node2Node_MessageID, siteOnOffData.toJsonString());
    LOG_INFO << "Publish node2node: " << siteOnOffData.toJsonString();
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
        //在本机发布站点下线消息
        qlibc::QData siteOnOffData;
        siteOnOffData.setString("message_id", Site_OnOffLine_MessageID);
        siteOnOffData.setValue("content", value);
        ServiceSiteManager::getInstance()->publishMessage(Site_OnOffLine_MessageID, siteOnOffData.toJsonString());
        LOG_INFO << "Publish onoffline: " << siteOnOffData.toJsonString();

        //向其它主机传送站点下线消息
        siteOnOffData.setString("message_id", Site_OnOff_Node2Node_MessageID);
        ServiceSiteManager::getInstance()->publishMessage(Site_OnOff_Node2Node_MessageID, siteOnOffData.toJsonString());
        LOG_INFO << "Publish node2node: " << siteOnOffData.toJsonString();
    }
}

//提取在线站点注册消息
qlibc::QData SiteTree::getLocalSiteInfo(string& siteId) {
    std::lock_guard<std::recursive_mutex> lg(siteMutex);
    qlibc::QData data(Json::arrayValue);
    if(siteId.empty()){
        for(auto& elem : localSiteMap){
            data.append(elem.second);
        }
    }else{
        auto pos = localSiteMap.find(siteId);
        if(pos != localSiteMap.end()){
            data.append(pos->second);
        }
    }
    return data;
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

//站点计数归一
void SiteTree::resetLocalSitePingCounter(string& siteId){
    std::lock_guard<std::recursive_mutex> lg(siteMutex);
    auto pos = localSitePingCountMap.find(siteId);
    if(pos != localSitePingCountMap.end()){
        pos->second = 1;
    }
}


string SiteTree::getLocalIpAddress() {
    std::lock_guard<std::recursive_mutex> lg(siteMutex);
    return localIp;
}

void SiteTree::addNewFindSite(string& ip) {
    bool flag = false;
    //如果是本机自己的回复，则返回
    {
        std::lock_guard<std::recursive_mutex> lg(siteMutex);
        auto pos = ipMap.find(ip);
        if(pos == ipMap.end()){     //不是本机自己回复
            flag = true;
        }
    }

    /*
     * . 获取相应主机下的站点信息，如果之前存在则更新，如果之前不存在则添加
     */
    if(flag && initComplete.load()){
        qlibc::QData request, response;
        request.setString("service_id", Site_LocalSite_Service_ID);
        request.putData("request", qlibc::QData());
        if(httpUtil::sitePostRequest(ip, 9000, request, response)){
            bool isNewNode = false;
            {
                std::lock_guard<std::recursive_mutex> lg(siteMutex);
                auto pos = discoveredSiteMap.find(ip);
                if(pos != discoveredSiteMap.end()){
                    discoveredSiteMap.erase(ip);
                }else{  //为新增节点，要发布节点上线消息
                    isNewNode = true;
                }
                discoveredSiteMap.insert(std::make_pair(ip, response.getData("response").getData("siteList").asValue()));

                auto pos1 = discoveredPingCountMap.find(ip);
                if(pos1 != discoveredPingCountMap.end()){
                    discoveredPingCountMap.erase(pos1);
                }
                discoveredPingCountMap.insert(std::make_pair(ip, 1));
            }

            //如果是新增节点则发布上线消息； 目前只关心本机站点的上下线
#if 0
            if(isNewNode){
                qlibc::QData siteList = response.getData("response").getData("siteList");
                publishOnOffLineMessage(siteList, "online", false);
            }
#endif

            //订阅节点消息通道，重复订阅也没有关系
            std::vector<string> messageIdList;
            messageIdList.push_back(Site_OnOff_Node2Node_MessageID);
            ServiceSiteManager::subscribeMessage(ip, 9000, messageIdList);
        }
    }
}


/*
 * 更新主机下挂的站点信息
 *      1. 删除下线站点
 *      2. 忽略本来在线的站点
 *      3. 增加新上线的站点
 */
void SiteTree::updateFindSite(qlibc::QData& siteInfo){
    LOG_INFO << "updateFindSite: " << siteInfo.toJsonString();
    //判断属于哪个节点
    string site_id = siteInfo.getString("site_id");
    string siteIp = siteInfo.getString("ip");
    string onoffline = siteInfo.getString("site_status");

    //找到信息所属的节点
    {
        std::lock_guard<std::recursive_mutex> lg(siteMutex);
        auto pos = discoveredSiteMap.find(siteIp);
        if(pos != discoveredSiteMap.end()){
            Json::Value list = pos->second;
            Json::ArrayIndex size = list.size();
            Json::ArrayIndex deleIndex = -1;

            for(Json::ArrayIndex i = 0; i < size; ++i){
                if(onoffline =="offline" && list[i]["site_id"] == site_id){
                    deleIndex = i;
                    break;
                }

                //已有上线站点不处理
                if(onoffline =="online" && list[i]["site_id"] == site_id){
                    LOG_PURPLE << "IGNORE SITE....";
                    return;
                }

                //增加上线站点
                if(onoffline == "online" && i == size -1){
                    pos->second.append(siteInfo.asValue());
                    LOG_PURPLE << "ADD SITE: <" << siteIp << ":" << site_id << ">";
                    return;
                }
            }

            //删除离线站点
            if(deleIndex != -1){
                Json::Value removeValue;
                pos->second.removeIndex(deleIndex, &removeValue);
                LOG_PURPLE << "DEL SITE: <" << siteIp << ":" << site_id << ">";
            }
        }
    }
}


qlibc::QData SiteTree::getLocalAreaSite(string& siteId){
    std::lock_guard<std::recursive_mutex> lg(siteMutex);
    qlibc::QData retData;
    if(!siteId.empty()){
        qlibc::QData localData;
        for(auto& elem : localSiteMap){
            if(elem.first == siteId){
                localData.append(elem.second);
                retData.putData(localIp, localData);
                break;
            }
        }

        for(auto& elem : discoveredSiteMap){
            Json::ArrayIndex size = elem.second.size();
            for(Json::ArrayIndex i = 0; i < size; ++i){
                if(elem.second[i]["site_id"] == siteId){
                    retData.setValue(elem.first, Json::Value(qlibc::QData().append(elem.second[i]).asValue()));
                    break;
                }
            }
        }

    }else{
        qlibc::QData localData;
        for(auto& elem : localSiteMap){
            localData.append(elem.second);
        }
        retData.putData(localIp, localData);

        for(auto& elem : discoveredSiteMap){
            retData.setValue(elem.first, elem.second);
        }
    }

    return retData;
}


qlibc::QData SiteTree::getLocalAreaSiteExceptOwn(string& siteId){
    std::lock_guard<std::recursive_mutex> lg(siteMutex);
    qlibc::QData retData;
    if(!siteId.empty()){
        for(auto& elem : discoveredSiteMap){
            Json::ArrayIndex size = elem.second.size();
            for(Json::ArrayIndex i = 0; i < size; ++i){
                if(elem.second[i]["site_id"] == siteId){
                    retData.setValue(elem.first, Json::Value(qlibc::QData().append(elem.second[i]).asValue()));
                    break;
                }
            }
        }

    }else{
        for(auto& elem : discoveredSiteMap){
            retData.setValue(elem.first, elem.second);
        }
    }

    return retData;
}

qlibc::QData SiteTree::printIpAddress(){
    std::lock_guard<std::recursive_mutex> lg(siteMutex);
    qlibc::QData data;
    data.setString("localIp", localIp);
    Json::Value array(Json::arrayValue);
    for(auto& elem : ipMap){
        array.append(elem.first);
    }
    data.setValue("ipset", array);
    return data;
}

int SiteTree::initLocalIp() {
    int sockets[32];
    int max_sockets = sizeof(sockets) / sizeof(sockets[0]);
    int num_sockets = 0;
    int retFlag = -1;

    struct ifaddrs* ifaddr = nullptr;
    struct ifaddrs* ifa = nullptr;
    if (getifaddrs(&ifaddr) < 0){   //获取所有网卡的网卡地址
        LOG_RED << "Unable to get interface addresses...";
        return -1;
    }

    // 遍历获取的本机网卡信息
    for (ifa = ifaddr; ifa; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr)  //网卡地址有效
            continue;
        if (!(ifa->ifa_flags & IFF_UP) || !(ifa->ifa_flags & IFF_MULTICAST))        //接口启用、支持组播
            continue;
        if ((ifa->ifa_flags & IFF_LOOPBACK) || (ifa->ifa_flags & IFF_POINTOPOINT))  //不是回环口地址、不是点对点网络
            continue;

        if (ifa->ifa_addr->sa_family == AF_INET) {
            struct sockaddr_in* saddr = (struct sockaddr_in*)ifa->ifa_addr;
            if (saddr->sin_addr.s_addr != htonl(INADDR_LOOPBACK)) {  //不是回环口地址
                int log_addr = 0;
                if (num_sockets < max_sockets) {
                    saddr->sin_port = htons(0);      //分配端口号
                    int sock = mdns_socket_open_ipv4(saddr);  //创建socket, 绑定该地址
                    if (sock >= 0) {
                        num_sockets++;
                        mdns_socket_close(sock);
                        log_addr = 1;
                    } else {
                        log_addr = 0;
                    }
                }

                if (log_addr) {
                    char buffer[128];
                    mdns_string_t addr = ipv4_address_to_string(buffer, sizeof(buffer), saddr,sizeof(struct sockaddr_in));
                    std::lock_guard<std::recursive_mutex> lg(siteMutex);
                    auto pos = ipMap.find(string(addr.str));
                    if(pos != ipMap.end()){
                        if(pos->second != string(ifa->ifa_name)){
                            ipMap.erase(pos);
                            ipMap.insert(std::make_pair(string(addr.str), string(ifa->ifa_name)));
                            LOG_PURPLE << "initLocalIp: " << string(addr.str) << " : " << string(ifa->ifa_name);
                        }
                    }else{
                        ipMap.insert(std::make_pair(string(addr.str), string(ifa->ifa_name)));
                        LOG_PURPLE << "initLocalIp: " << string(addr.str) << " : " << string(ifa->ifa_name);
                    }

                    if(regex_search(string(ifa->ifa_name), regex("wlan")) || regex_search(string(ifa->ifa_name), regex("eth"))){
                        //如果本机IP值发生变化（一般是断网重连造成的），则删除旧的本地IP值
                        if(localIp != string(addr.str)){
                            auto position = ipMap.find(localIp);
                            if(position != ipMap.end()){
                                ipMap.erase(position);
                            }
                            localIp = string(addr.str);       // 更新本机IP地址
                            updateLocalSiteIp();             // 更新本机站点信息中的Ip地址信息
                            LOG_PURPLE << "localIp: " << localIp;

                            std::lock_guard<std::mutex> lgMdns(mdnsMutex);
                            ready2StartMdnsService = true;
                            cv.notify_one();     //通知mdns服务启动
                        }
                        retFlag = 0;
                    }
                }
            }
        }
    }

    freeifaddrs(ifaddr);
    return retFlag;
}

void SiteTree::insertLocalQuerySiteInfo() {
    std::lock_guard<std::recursive_mutex> lg(siteMutex);
    Json::Value querySiteInfo;
    querySiteInfo["ip"] = localIp;
    querySiteInfo["port"] = 9000;
    querySiteInfo["site_id"] = "site_query";
    querySiteInfo["site_status"] = "online";
    querySiteInfo["summary"] = "服务站点查询";
    localSiteMap.insert(std::make_pair("site_query", querySiteInfo));
    localSitePingCountMap.insert(std::make_pair("site_query", 1));
}

//站点计数-1，移除离线站点计数、发布站点离线消息，通过节点通道发送该消息
void SiteTree::localSitePingCountDown(){
    qlibc::QData offlineList(Json::arrayValue);
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
                    offlineList.append(elemPos->second);
                    localSiteMap.erase(elemPos);
                }
                //移除离线站点的计数
                pos = localSitePingCountMap.erase(pos);
            }else{
                pos++;
            }
        }
    }
    //发布离线站点消息
    publishOnOffLineMessage(offlineList, "offline", true);
}

/*
 *  先前发现的站点连续3次都没有发现
 */
void SiteTree::discoveredSitePingCountDown(){
    qlibc::QData offLineData;
    {
        std::lock_guard<std::recursive_mutex> lg(siteMutex);
        for(auto& elem : discoveredPingCountMap){
            elem.second += -1;
        }

        for(auto pos = discoveredPingCountMap.begin(); pos != discoveredPingCountMap.end();){
            if(pos->second <= -3){
                auto elemPos = discoveredSiteMap.find(pos->first);
                if(elemPos != discoveredSiteMap.end()){
                    offLineData.setValue(elemPos->first, elemPos->second);
                    discoveredSiteMap.erase(elemPos);
                }
                //移除离线站点的计数
                pos = discoveredPingCountMap.erase(pos);
            }else{
                pos++;
            }
        }
    }

#if 0
    //监测到节点掉线后，发布节点下挂的站点的离线消息
    Json::Value::Members keys = offLineData.getMemberNames();
    for(auto& key : keys){
        qlibc::QData offList = offLineData.getData(key);
        publishOnOffLineMessage(offList, "offline", false);
    }
#endif
}


void SiteTree::site_query() {
    string querySiteName = "_edgeai.site_query._tcp.local.";
    mdns_query_t query[1];
    query[0].name = querySiteName.c_str();
    query[0].length = strlen(query[0].name);
    query[0].type = MDNS_RECORDTYPE_PTR;
    send_mdns_query(query, 1);
}

void SiteTree::mdnsServiceStart(){
    {
        std::unique_lock<std::mutex> ul(mdnsMutex);
        cv.wait(ul, [this]{return this->ready2StartMdnsService;});
    }
    std::this_thread::sleep_for(std::chrono::seconds(3));
    LOG_BLUE << "START MDNS SERVICE.....";

    string hostname = "smartHome";
    char hostname_buffer[256];
    size_t hostname_size = sizeof(hostname_buffer);
    if (gethostname(hostname_buffer, hostname_size) == 0){
        hostname = hostname_buffer;
    }

    //mdns服务器监听5353端口号，此处指定请求服务名以及返回的端口号
    string service = "edgeai.site_query._tcp.local.";
    int service_port = 9000;
    while(true){
        service_mdns(hostname.c_str(), service.c_str(), service_port);
        LOG_RED << "failed to start mdnsService, start again in 3 seconds.....";
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
}

void SiteTree::updateLocalSiteIp(){
    std::lock_guard<std::recursive_mutex> lg(siteMutex);
    for(auto& elem : localSiteMap){
        elem.second["ip"] = localIp;
    }
}

void SiteTree::publishOnOffLineMessage(qlibc::QData& siteList, string onOffLine, bool is2Node){
    Json::ArrayIndex offListSize = siteList.size();
    for(int i = 0; i < offListSize; i++){
        qlibc::QData ithData = siteList.getArrayElement(i);
        ithData.setString("site_status", onOffLine);
        qlibc::QData siteOnOffData;
        siteOnOffData.setString("message_id", Site_OnOffLine_MessageID);
        siteOnOffData.setValue("content", ithData.asValue());
        ServiceSiteManager::getInstance()->publishMessage(Site_OnOffLine_MessageID, siteOnOffData.toJsonString());
        LOG_INFO << "Publish onoffline: " << siteOnOffData.toJsonString();

        if(is2Node){
            siteOnOffData.setString("message_id", Site_OnOff_Node2Node_MessageID);
            ServiceSiteManager::getInstance()->publishMessage(Site_OnOff_Node2Node_MessageID, siteOnOffData.toJsonString());
            LOG_INFO << "Publish node2node: " << siteOnOffData.toJsonString();
        }
    }
}


//处理mdns请求返回，发布站点信息
void mdnsResponseHandle(string service_instance_string, string ipString, int sitePort){
    smatch sm, ipSm;
    bool matchRet = regex_match(service_instance_string, sm, regex("(.*)[.](.*)[.](.*)[.](.*)[.](.*)[.]"));
    if(matchRet){
        string site_id = sm.str(3);
        if(regex_match(ipString, ipSm, regex("(.*):(.*)"))){
            string ip = ipSm.str(1);

            //依据节点的查询站点回复来更新节点信息
            if(site_id == "site_query"){
                //更新局域网发现的站点
                SiteTree::getInstance()->addNewFindSite(ip);
            }

            //发布站点发布消息
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
