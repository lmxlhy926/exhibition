//
// Created by 78472 on 2022/8/12.
//

#include "msgSubscribeHandler.h"
#include "siteService/service_site_manager.h"
#include "qlibc/QData.h"
#include "param.h"
#include "common/httpUtil.h"
#include "log/Logging.h"


using namespace servicesite;

void synergy::register2QuerySite(const Request& request) {
    LOG_HLIGHT << "==>receive registerAgain message, start to register again.....";
    qlibc::QData data;
    qlibc::QData registerData;
    qlibc::QData response;
    registerData.setString("site_id", SYNERGY_SITE_ID);
    registerData.setInt("port", SynergySitePort);
    registerData.setString("summary", SYNERGY_SITE_ID_NAME);
    data.setString("service_id", "site_register");
    data.putData("request", registerData);

    while (true) {
        httpUtil::sitePostRequest(RequestIp, QuerySitePort, data, response);

        if (response.getInt("code") == 0) {
            LOG_HLIGHT << "==>register to QuerySite successfully";
            break;
        } else {
            LOG_RED << "==>register to QuerySite failed.......";
            LOG_RED << "==>start to register agagin in 3 seconds.......";
            this_thread::sleep_for(std::chrono::seconds(3));
        }
    }
}