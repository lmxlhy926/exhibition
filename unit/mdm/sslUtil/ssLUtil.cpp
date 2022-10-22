//
// Created by 78472 on 2022/10/22.
//

#include "ssLUtil.h"

ssLUtil::SingleSite::SingleSite(string ip, int port) {

}

bool ssLUtil::SingleSite::send(qlibc::QData &request, qlibc::QData &response) {
    return false;
}

void ssLUtil::SingleSite::deleteClient() {

}

string ssLUtil::SingleSite::getSiteIp() {
    return std::string();
}

int ssLUtil::SingleSite::getSitePort() {
    return 0;
}
