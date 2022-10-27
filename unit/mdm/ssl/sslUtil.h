//
// Created by 78472 on 2022/10/22.
//

#ifndef EXHIBITION_SSLUTIL_H
#define EXHIBITION_SSLUTIL_H

#include <string>
#include "http/httplib.h"
#include "qlibc/QData.h"

using namespace std;
using namespace httplib;

class sslUtil {
private:
    class SingleSite{
    private:
        string host_;               //服务器地址
        string ca_cert_path_;       //认证证书地址
        httplib::SSLClient* clientptr = nullptr;
    public:
        explicit SingleSite(string host, string ca_cert_path);

        //云端发送https请求
        bool send(string &uri, qlibc::QData& request, qlibc::QData& response);

        //释放客户端
        void deleteClient();
    };

    using SingleSiteVec = std::map<string, SingleSite>;

private:
    SingleSiteVec sites;
    std::recursive_mutex rMutex;
    static sslUtil* instance;
public:
    static sslUtil* getInstance();

    //添加一个云端服务
    void addCloudSite(string cloudService, string& host, string& ca_cert_path);

    //发送一个请求
    bool sendRequest2Site(const string& cloudService, string uri, qlibc::QData& request, qlibc::QData& response);
};


#endif //EXHIBITION_SSLUTIL_H
