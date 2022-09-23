//
// Created by 78472 on 2022/5/18.
//

#ifndef EXHIBITION_HTTPUTIL_H
#define EXHIBITION_HTTPUTIL_H

#include <string>
#include "httplib.h"
#include "qlibc/QData.h"
#include <vector>

using namespace httplib;
using namespace std;

class httpUtil {
public:
    static bool sitePostRequest(const string& ip, int port, qlibc::QData& request, qlibc::QData& response);

};

class SingleSite{
private:
    string siteIp;
    int    sitePort;
    Client* clientptr = nullptr;
public:
    explicit SingleSite(string ip, int port);

    //向站点发送请求
    bool send(qlibc::QData& request, qlibc::QData& response);

    //释放客户端
    void deleteClient();

    string getSiteIp();

    int getSitePort();
};

using SingleSiteVec = std::map<string, SingleSite>;
class SiteRecord{
private:
    SingleSiteVec sites;
    std::recursive_mutex rMutex;
    static SiteRecord* instance;
public:
    static SiteRecord* getInstance();

    void addSite(string siteName, string siteIp, int sitePort);

    bool sendRequest2Site(string siteName, qlibc::QData& request, qlibc::QData& response);

    void printMap();
};

#endif //EXHIBITION_HTTPUTIL_H
