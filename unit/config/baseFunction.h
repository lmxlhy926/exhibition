//
// Created by 78472 on 2022/5/10.
//

#ifndef EXHIBITION_BASEFUNCTION_H
#define EXHIBITION_BASEFUNCTION_H


#include <string>
#include "qlibc/QData.h"
#include "socket/httplib.h"

//向云端发送加密请求
bool ecb_httppost(httplib::Client *client, const std::string& uri,
                  const qlibc::QData &request, qlibc::QData &response);



bool joinTvWhite(const string& cacheDir);





#endif //EXHIBITION_BASEFUNCTION_H
