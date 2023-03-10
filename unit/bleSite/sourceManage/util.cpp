//
// Created by 78472 on 2023/3/6.
//

#include "util.h"
#include "qlibc/QData.h"
#include "common/httpUtil.h"
#include "log/Logging.h"
#include "../parameter.h"


bool getSynergyInfo(string& ip, int& port){
    qlibc::QData appSiteRequest, appSiteResponse;
    appSiteRequest.setString("service_id", "get_app_site_list");
    appSiteRequest.setValue("request", Json::Value("{}"));
    bool siteBool = httpUtil::sitePostRequest("127.0.0.1", 9012, appSiteRequest, appSiteResponse);
    LOG_INFO << "appSiteResponse: " << appSiteResponse.toJsonString();

//    siteBool = true;
//    appSiteResponse.loadFromFile("/mnt/d/bywg/project/exhibition/unit/bleSite/sourceManage/appSite.json");

    if(siteBool) {
        qlibc::QData siteList = appSiteResponse.getData("response").getData("app_site_list");
        Json::ArrayIndex siteListSize = siteList.size();
        for (Json::ArrayIndex i = 0; i < siteListSize; ++i) {
            qlibc::QData item = siteList.getArrayElement(i);
            if (item.getString("site_id") == "collaborate") {
                ip = item.getString("ip");
                port = item.getInt("port");
                LOG_PURPLE << "synergy: " << ip << ", " << port;
                return true;
            }
        }
    }
    return false;
}


void util::updateDeviceList(){
    string ip;
    int port;
    if(getSynergyInfo(ip, port)){
        qlibc::QData request, response;
        request.setString("service_id", "updateDeviceList");
        request.putData("request", qlibc::QData());
        if(httpUtil::sitePostRequest(ip, port, request, response)){
            if(response.getInt("code") == 0){
                LOG_PURPLE << "==>notify synergy to updateDeviceList successfully.......";
            }else{
                LOG_RED << "==>notify synergy to updateDeviceList failed.......";
            }
        }else{
            LOG_RED << "==>notify synergy to updateDeviceList failed, cangt access synergy site.......";
        }
    }else{
        LOG_RED << "==>notify synergy to updateDeviceList failed, cannot get synergy site.......";
    }
}

void util::updateWhiteDeviceList() {
    LOG_INFO << "==>notify synergy to updateWhiteDeviceList....";
    string ip;
    int port;
    if(getSynergyInfo(ip, port)){
        qlibc::QData request, response;
        request.setString("service_id", "updateWhiteList");
        request.putData("request", qlibc::QData());
        if(httpUtil::sitePostRequest(ip, port, request, response)){
            if(response.getInt("code") == 0){
                LOG_PURPLE << "==>synergy updateWhiteDeviceList successfully, " << response.getString("msg") << ".......";
            }else{
                LOG_RED << "==>synergy updateWhiteDeviceList failed, " << response.getString("msg") << ".......";
            }
        }else{
            LOG_RED << "==>synergy updateWhiteDeviceList failed, cant access synergy site.......";
        }
    }else{
        LOG_RED << "==>synergy updateWhiteDeviceList failed, cannot get synergy site.......";
    }
}