//
// Created by WJG on 2022-4-25.
//

#include <iostream>
#include <dirent.h>
#include <string.h>
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>
#include "SiteTree.h"
#include "comm_define.h"
#include "StringUtils.h"
#include "log_tool.h"

std::string SiteTree::config_file_dir = "";
SiteTree* SiteTree::m_pInstance = new SiteTree();
SiteTree::SiteTree(){
}

SiteTree::~SiteTree()
{
    allSite.clear();
}

int SiteTree::initSiteTree(const std::string &file_dir){
    if(file_dir.empty())
        return 1;

    config_file_dir = file_dir;
    used_port = 9000;

    return 0;
}

Json::Value SiteTree::getAllSite(){
    Json::Value value;
    std::unordered_map<std::string, Json::Value>::const_iterator iter;
    treeMutex.lock();
    for(iter = allSite.begin(); iter != allSite.end(); ++iter)
    {
        value.append(iter->second);
    }
    treeMutex.unlock();

    return value;
}

std::string SiteTree::getSiteInfo(const std::string &site_id, const std::string &key){
    std::unordered_map<std::string, Json::Value>::const_iterator iter;
    treeMutex.lock();
    iter = allSite.find(site_id);
    treeMutex.unlock();
    if(iter != allSite.end())
    {
        Json::Value sobj = iter->second;
        return getInfo(sobj, key);
    }
    return "";
}

Json::Value SiteTree::getSiteAllInfo(const std::string &site_id){
    std::unordered_map<std::string, Json::Value>::const_iterator iter;
    treeMutex.lock();
    iter = allSite.find(site_id);
    treeMutex.unlock();
    if(iter != allSite.end())
    {
        Json::Value sobj = iter->second;
        return sobj;
    }

    return Json::nullValue;
}

int SiteTree::updateSiteInfo(const std::string &site_id, const Json::Value &info){
    if(site_id.empty() || info.empty())
        return 1;


    std::unordered_map<std::string, Json::Value>::iterator iter;
    treeMutex.lock();
    iter = allSite.find(site_id);
    if(iter != allSite.end())
    {
        Json::Value &sobj = iter->second;
        setInfo(sobj, info);
    }
    else
    {
        Json::Value sobj;
        setInfo(sobj, SITE_ID, site_id);
        setInfo(sobj, info);
        allSite.insert(std::pair<std::string, Json::Value>(site_id, sobj));
    }
    treeMutex.unlock();

    saveSiteInfo(site_id);
    return 0;
}

int SiteTree::updateSiteInfo(const std::string &site_id, const std::string &key, const std::string &value){
    if(site_id.empty() || key.empty() || value.empty())
        return 1;

    Json::Value jObject;
    jObject[key] = value;
    return updateSiteInfo(site_id, jObject);
}

int SiteTree::cacheSiteInfo(const std::string &site_id, const Json::Value &info){
    if(site_id.empty() || info.empty())
        return 1;

    int ret = updateSiteInfo(site_id, info);
    site_id_setlist.insert(site_id);

    return ret;
}

int SiteTree::refreshSitePingCounter(const std::string &site_id, int mark){
    if(site_id.empty())
        return 0;

    int ret = 0;
    pingMutex.lock();

    auto item = site_ping_counter.find(site_id);
    if(item == site_ping_counter.end())
    {
        site_ping_counter.insert(std::pair<std::string,int>(site_id, 1));
        ret = 1;
    }
    else
    {
        if(mark == 1)
            item->second += 1;
        else if(mark == 0)
            item->second -= 1;

        ret = item->second;
    }

    if(ret <= -3)
        site_ping_counter.erase(site_id);

    pingMutex.unlock();

    return ret;
}

std::unordered_set<std::string> SiteTree::getSiteIdList(){
    return site_id_setlist;
}

int SiteTree::notifySiteOffline(const std::string &site_id){
    if(!site_id.empty())
    {
        deleteSite(site_id);
        site_id_setlist.erase(site_id);
    }
    return 0;
}

std::unordered_set<std::string> SiteTree::getSubscribeSite(const std::string msg_id){
    std::unordered_set<std::string> r_set;

    if(!msg_id.empty())
    {
        auto item = subscribe_site_map.find(msg_id);
        if(item != subscribe_site_map.end())
            r_set = item->second;
    }
    return r_set;
}

bool SiteTree::hasSite(const std::string &site_id){
    if(site_id.empty())
        return false;

    std::unordered_map<std::string, Json::Value>::const_iterator iter;
    treeMutex.lock();
    iter = allSite.find(site_id);
    treeMutex.unlock();
    if(iter != allSite.end())
        return true;

    return false;
}

int SiteTree::deleteSite(const std::string &site_id){
    std::unordered_map<std::string, Json::Value>::const_iterator iter;

    treeMutex.lock();

    iter = allSite.find(site_id);
    if(iter != allSite.end())
        allSite.erase(site_id);  //从站点树中删除对应站点

    treeMutex.unlock();
    return 0;
}

void SiteTree::saveSiteInfo(std::string site_id){
    if(site_id.empty())
        return;
}

int SiteTree::getSiteport(){
    int tmp_port = used_port+1;
    used_port = tmp_port;
    return tmp_port;
}

void SiteTree::setInfo(Json::Value &sobj, const std::string &key, const std::string &value){
    if(!key.empty() && !value.empty())
        sobj[key] = value;
}
void SiteTree::setInfo(Json::Value &sobj, const Json::Value &value){
    if(!value.empty())
    {
        Json::Value::Members members = value.getMemberNames();
        Json::Value::Members::iterator iter;
        for (iter = members.begin(); iter != members.end(); ++iter)
            sobj[*iter] =  value[*iter];
    }
}

std::string SiteTree::getInfo(const Json::Value &sobj, const std::string &key){
    if(!key.empty())
    {
       Json::Value value = sobj.get(key, Json::nullValue);
       if(value.empty())
           return "";

       std::string str;
       Json::ValueType vt = value.type();
       switch (vt)
       {
           case Json::stringValue:
           {
               str = value.asString();
               break;
           }
           case Json::intValue:
           case Json::uintValue:
           case Json::realValue:
           case Json::booleanValue:
           {
               int intTmp = value.asInt();
               str = std::to_string(intTmp);
               break;
           }
           default:
               break;
       }//end switch
        return str;
    }

    return "";
}
