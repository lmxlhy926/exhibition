//
// Created by 78472 on 2023/2/21.
//


#include "fileSync.h"
#include "qlibc/QData.h"
#include "common/httpUtil.h"
#include "../param.h"
#include "log/Logging.h"
#include "configParamUtil.h"
#include "siteService/service_site_manager.h"
#include <regex>
using namespace servicesite;

void fileSync::subscribeOtherConfigSites() {
    updateConfigSites();
    std::set<string > siteNames = SiteRecord::getInstance()->getSiteName();
    for(auto& siteName : siteNames){
        smatch sm;
        if(regex_match(siteName, sm,regex("(.*):config"))){  //本机外的其它配置站点
            std::vector<string> messageIdList;
            messageIdList.push_back(FileSync_MESSAGE_ID);
            ServiceSiteManager::subscribeMessage(sm.str(1), ConfigSitePort, messageIdList);
        }
    }
}

void fileSync::publishSyncMessages() {
    qlibc::QData data;
    data.setString("fileName", "whiteList");
    data.setInt("updateTimeStamp", configParamUtil::getInstance()->getWhiteList().getInt("updateTimeStamp"));
    ServiceSiteManager::getInstance()->publishMessage(FileSync_MESSAGE_ID, data.toJsonString());

    data.setString("fileName", "rules");
    data.setInt("updateTimeStamp", configParamUtil::getInstance()->getSceneConfigFile().getInt("updateTimeStamp"));
    ServiceSiteManager::getInstance()->publishMessage(FileSync_MESSAGE_ID, data.toJsonString());
}

void fileSync::updateFile(const Request &request) {
    qlibc::QData data(request.body);
    string fileName = data.getData("content").getString("fileName");
    int updateTimeStamp = data.getData("content").getInt("updateTimeStamp");

    int fileTimeStamp = 0;
    if(fileName == "whiteList"){
        fileTimeStamp = configParamUtil::getInstance()->getWhiteList().getInt("updateTimeStamp");
        if(fileTimeStamp < updateTimeStamp){
            qlibc::QData requestData, responseData;
            requestData.setString("service_id", WHITELIST_REQUEST_SERVICE_ID);
            requestData.putData("request", qlibc::QData());
            if(httpUtil::sitePostRequest(request.remote_addr, ConfigSitePort, requestData, responseData)){
                qlibc::QData whiteList = responseData.getData("response");
                configParamUtil::getInstance()->saveWhiteListData(whiteList);
            }
        }
    }else if(fileName == "rules"){
        fileTimeStamp = configParamUtil::getInstance()->getSceneConfigFile().getInt("updateTimeStamp");
        if(fileTimeStamp < updateTimeStamp){
            qlibc::QData requestData, responseData;
            requestData.setString("service_id", GETSCENECONFIGFILE_REQUEST_SERVICE_ID);
            requestData.putData("request", qlibc::QData());
            if(httpUtil::sitePostRequest(request.remote_addr, ConfigSitePort, requestData, responseData)){
                qlibc::QData sceneData = responseData.getData("response");
                configParamUtil::getInstance()->saveSceneConfigFile(sceneData);
            }
        }
    }
}

void fileSync::updateConfigSites() {
    qlibc::QData request, response;
    request.setString("service_id", "site_localAreaNetworkSiteExceptLocal");
    request.putData("request", qlibc::QData().setString("site_id", ""));
    if(httpUtil::sitePostRequest("127.0.0.1", QuerySitePort, request, response)){    //获取局域网内所有发现的站点
        qlibc::QData resBody = response.getData("response");
        Json::Value::Members members = resBody.getMemberNames();
        for(auto& elem : members){
            qlibc::QData siteList = resBody.getData(elem);
            Json::ArrayIndex size = siteList.size();
            for(Json::ArrayIndex i = 0; i < size; ++i){
                qlibc::QData item = siteList.getArrayElement(i);
                string site_id = item.getString("site_id");
                if(site_id == CONFIG_SITE_ID){
                    string ipSiteName = elem + ":" + site_id;   //ip:siteID
                    SiteRecord::getInstance()->addSite(ipSiteName, elem, item.getInt("port"));  //记录选定的站点
                }
            }
        }
    }
}


