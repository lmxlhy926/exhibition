//
// Created by 78472 on 2023/3/4.
//

#include "siteManager.h"
#include "common/httpUtil.h"
#include "../param.h"
#include "log/Logging.h"

void siteManager::updateSiteBak() {
    qlibc::QData request, response;
    request.setString("service_id", "site_localAreaNetworkSite");
    request.putData("request", qlibc::QData().setString("site_id", ""));
    if(httpUtil::sitePostRequest("127.0.0.1", 9000, request, response)){    //获取局域网内所有发现的站点
        qlibc::QData resBody = response.getData("response");
        Json::Value::Members members = resBody.getMemberNames();
        for(auto& elem : members){
            qlibc::QData siteList = resBody.getData(elem);
            Json::ArrayIndex size = siteList.size();
            for(Json::ArrayIndex i = 0; i < size; ++i){
                qlibc::QData item = siteList.getArrayElement(i);
                string site_id = item.getString("site_id");
                if(site_id == BleSiteID || site_id == TvAdapterSiteID || site_id == ZigbeeSiteID){
                    string ipSiteName = elem + ":" + site_id;   //ip:siteID
                    SiteRecord::getInstance()->addSite(ipSiteName, elem, item.getInt("port"));  //记录选定的站点
                }
            }
        }
    }
}


qlibc::QData siteManager::updateSite() {
    qlibc::QData totalList;
    qlibc::QData appSiteRequest, nodeSiteRequest;
    appSiteRequest.setString("service_id", "get_app_site_list");
    appSiteRequest.putData("request", qlibc::QData());
    qlibc::QData appSiteResponse, nodeSiteResponse;
    nodeSiteRequest.setString("service_id", "get_all");
    nodeSiteRequest.putData("request", qlibc::QData());

    bool siteBool = httpUtil::sitePostRequest("127.0.0.1", 9012, appSiteRequest, appSiteResponse);
    bool nodeBool = httpUtil::sitePostRequest("127.0.0.1", 9012, nodeSiteRequest, nodeSiteResponse);

//测试-----------
    siteBool = true;
    nodeBool = true;
    appSiteResponse.loadFromFile("/mnt/d/bywg/project/exhibition/unit/synergy/sourceManage/appSite.json");
    nodeSiteResponse.loadFromFile("/mnt/d/bywg/project/exhibition/unit/synergy/sourceManage/nodeSite.json");

//测试-----------
    if(siteBool && nodeBool) {    //整合站点列表
        qlibc::QData siteList = appSiteResponse.getData("response").getData("app_site_list");
        Json::ArrayIndex siteListSize = siteList.size();
        qlibc::QData nodeList = nodeSiteResponse.getData("response").getData("node_list");
        Json::ArrayIndex nodeListSize = nodeList.size();
        for (Json::ArrayIndex i = 0; i < siteListSize; ++i) {
            qlibc::QData siteItem = siteList.getArrayElement(i);
            for (Json::ArrayIndex j = 0; j < nodeListSize; ++j) {
                qlibc::QData nodeItem = nodeList.getArrayElement(j);
                if (siteItem.getString("ip") == nodeItem.getString("ip")) {
                    siteItem.setString("uid", nodeItem.getString("uid"));
                    totalList.append(siteItem);
                    break;
                }
            }
        }
    }

    Json::ArrayIndex totalListSize = totalList.size();
    for(Json::ArrayIndex t = 0; t < totalListSize; ++t){
        qlibc::QData tItem = totalList.getArrayElement(t);
        string site_id = tItem.getString("site_id");
        if(site_id == BleSiteID || site_id == TvAdapterSiteID || site_id == ZigbeeSiteID){
            string siteIndex;
            siteIndex.append(tItem.getString("uid").append(":").append(tItem.getString("site_id")));    //uid:site_id
            SiteRecord::getInstance()->addSite(siteIndex, tItem.getString("ip"), tItem.getInt("port"));
        }
    }

//    SiteRecord::getInstance()->printMap();

    return totalList;
}
