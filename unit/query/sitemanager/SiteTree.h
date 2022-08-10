//
// Created by WJG on 2022-4-25.
//

#ifndef SITEMANAGERSERVICE_SITETREE_H
#define SITEMANAGERSERVICE_SITETREE_H

#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include "qlibc/jsoncpp/json.h"
#include "comm_define.h"

class SiteTree{
public:
    static SiteTree& GetInstance()
    {
        if ( m_pInstance == nullptr )
            m_pInstance = new SiteTree();
        return *m_pInstance;
    };

    ~SiteTree();

    static std::string config_file_dir;

    int initSiteTree(const std::string &file_dir);
    Json::Value getAllSite();
    std::string getSiteInfo(const std::string &site_id, const std::string &key);
    Json::Value getSiteAllInfo(const std::string &site_id);
    int updateSiteInfo(const std::string &site_id, const Json::Value &info);
    int updateSiteInfo(const std::string &site_id, const std::string &key, const std::string &info);
    bool hasSite(const std::string &site_id);
    int cacheSiteInfo(const std::string &site_id, const Json::Value &info);
    int getSiteport();
    std::unordered_set<std::string> getSubscribeSite(const std::string msg_id);

    int refreshSitePingCounter(const std::string &site_id, int mark);
    std::unordered_set<std::string> getSiteIdList();
    int notifySiteOffline(const std::string &site_id);

private:

    int used_port;

    static SiteTree* m_pInstance;
    std::mutex treeMutex;

    //unordered_map<site_id，站点信息Json>
    std::unordered_map<std::string, Json::Value> allSite;

    //unordered_set<site_id>  已注册的站点
    std::unordered_set<std::string> site_id_setlist;

    std::mutex pingMutex;
    //unordered_map<site_id, ping_counter>  已注册的站点ping计数
    std::unordered_map<std::string, int> site_ping_counter;

    //unordered_map<订阅的msg_id，unordered_set<订阅此消息的site_id>>
    std::unordered_map<std::string, std::unordered_set<std::string>> subscribe_site_map;

    SiteTree();
//    std::mutex siteFileMutex;

    void setInfo(Json::Value &sobj, const Json::Value &value);
    void setInfo(Json::Value &sobj, const std::string &key, const std::string &value);
    std::string getInfo(const Json::Value &sobj, const std::string &key);
    void saveSiteInfo(std::string site_id);

    int deleteSite(const std::string &site_id);
};


#endif //SITEMANAGERSERVICE_SITETREE_H
