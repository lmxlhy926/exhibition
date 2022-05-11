//
// Created by 78472 on 2022/5/11.
//

#ifndef EXHIBITION_CLOUDUTIL_H
#define EXHIBITION_CLOUDUTIL_H

#include <string>
#include "qlibc/QData.h"
#include "socket/httplib.h"

using namespace std;

class cloudUtil {
private:
    const string serverIp;
    const int serverPort;
    const string dataDirPath;

public:
    explicit cloudUtil(string  ip, int port, string  dataDirectoryPath);

    bool joinTvWhite();

    bool tvRegister(qlibc::QData& engineerInfo);

    bool ecb_httppost(httplib::Client& client, const std::string& uri,
                      const qlibc::QData &request, qlibc::QData &response);
};


#endif //EXHIBITION_CLOUDUTIL_H
