//
// Created by 78472 on 2022/5/18.
//

#include "httpUtil.h"
#include "log/Logging.h"

bool httpUtil::sitePostRequest(const string& ip, int port, qlibc::QData& request, qlibc::QData& response){
    httplib::Client client(ip, port);
    httplib::Result result =  client.Post("/", request.toJsonString(), "text/json");
    if(result != nullptr){
        response.setInitData(qlibc::QData(result.value().body));
        return true;
    }
    return false;
}


SingleSite::SingleSite(string ip, int port) {
    siteIp = std::move(ip);
    sitePort = port;
}

bool SingleSite::send(qlibc::QData &request, qlibc::QData &response) {
    if(clientptr == nullptr){
       clientptr = new Client(siteIp, sitePort);
       clientptr->set_keep_alive(true);
       clientptr->set_connection_timeout(1, 0);
       clientptr->set_read_timeout(30, 0);
    }

    if(!clientptr->is_socket_open()){
       clientptr->stop();
       delete clientptr;
       clientptr = nullptr;

       clientptr = new Client(siteIp, sitePort);
       clientptr->set_keep_alive(true);
       clientptr->set_connection_timeout(1, 0);
       clientptr->set_read_timeout(30, 0);
    }

    httplib::Result result =  clientptr->Post("/", request.toJsonString(), "text/json");
    if(result != nullptr){
       response.setInitData(qlibc::QData(result.value().body));
       return true;
    }
    return false;
}

void SingleSite::deleteClient(){
    if(clientptr != nullptr){
        clientptr->stop();
        delete clientptr;
        clientptr = nullptr;
    }
}

string SingleSite::getSiteIp(){
    return siteIp;
}

int SingleSite::getSitePort(){
    return sitePort;
}

SiteRecord* SiteRecord::instance = nullptr;

 SiteRecord *SiteRecord::getInstance() {
    if(instance == nullptr){
        instance = new SiteRecord();
    }
    return instance;
}

void SiteRecord::addSite(string siteName, string siteIp, int sitePort) {
    std::lock_guard<std::recursive_mutex> lg(rMutex);
    auto pos = sites.find(siteName);
    if(pos != sites.end()){
        if(pos->second.getSiteIp() == siteIp && pos->second.getSitePort() == sitePort){
            return;
        }else{
            pos->second.deleteClient();
            sites.erase(siteName);
            sites.emplace(siteName, SingleSite(siteIp, sitePort));
        }
    }else{
        sites.emplace(siteName, SingleSite(siteIp, sitePort));
    }
}

void SiteRecord::removeSite(string siteName) {
    std::lock_guard<std::recursive_mutex> lg(rMutex);
    auto pos = sites.find(siteName);
    if(pos != sites.end()){
        pos->second.deleteClient();
        sites.erase(pos);
    }
}

bool SiteRecord::sendRequest2Site(string siteName, qlibc::QData &request, qlibc::QData &response) {
    std::lock_guard<std::recursive_mutex> lg(rMutex);
    auto pos = sites.find(siteName);
    if(pos != sites.end()){
        return pos->second.send(request, response);
    }
    return false;
}

void SiteRecord::printMap() {
    std::lock_guard<std::recursive_mutex> lg(rMutex);
    LOG_INFO << "sites:";
    for(auto& elem :sites){
        LOG_INFO << elem.first << ": <" << elem.second.getSiteIp() << ", " << elem.second.getSitePort() << ">";
    }
}

std::set<string> SiteRecord::getSiteName() {
    std::lock_guard<std::recursive_mutex> lg(rMutex);
    std::set<string> siteNameSet;
    for(auto& elem : sites){
       siteNameSet.insert(elem.first);
    }
    return siteNameSet;
}

