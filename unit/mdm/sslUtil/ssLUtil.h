//
// Created by 78472 on 2022/10/22.
//

#ifndef EXHIBITION_SSLUTIL_H
#define EXHIBITION_SSLUTIL_H

#include <string>
#include "common/httplib.h"
#include "qlibc/QData.h"

using namespace std;
using namespace httplib;

class ssLUtil {
private:
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

};


#endif //EXHIBITION_SSLUTIL_H
