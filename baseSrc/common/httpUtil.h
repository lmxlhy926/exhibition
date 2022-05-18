//
// Created by 78472 on 2022/5/18.
//

#ifndef EXHIBITION_HTTPUTIL_H
#define EXHIBITION_HTTPUTIL_H

#include <string>
#include "socket/httplib.h"
#include "qlibc/QData.h"

using namespace httplib;
using namespace std;

class httpUtil {
public:
    static bool sitePostRequest(const string& ip, int port, qlibc::QData& request, qlibc::QData& response);

};


#endif //EXHIBITION_HTTPUTIL_H
