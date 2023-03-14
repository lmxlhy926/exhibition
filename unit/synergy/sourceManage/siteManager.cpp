//
// Created by 78472 on 2023/3/4.
//

#include "siteManager.h"
#include "common/httpUtil.h"
#include "../param.h"
#include "log/Logging.h"
#include "siteService/service_site_manager.h"
using namespace std;
using namespace servicesite;
using namespace httplib;
using namespace std::placeholders;


void siteManager::updateSite(){
    std::map<string, Json::Value> sitesMap;
    qlibc::QData request, response;
    request.setString("service_id", "site_localAreaNetworkSite");
    request.putData("request", qlibc::QData().setString("site_id", ""));
    if(httpUtil::sitePostRequest("127.0.0.1", 9000, request, response)){    //获取局域网内所有发现的站点
        qlibc::QData resBody = response.getData("response");
        Json::Value::Members ipMembers = resBody.getMemberNames();
        for(auto& ip : ipMembers){
            qlibc::QData siteList = resBody.getData(ip);
            Json::ArrayIndex size = siteList.size();
            for(Json::ArrayIndex i = 0; i < size; ++i){
                qlibc::QData item = siteList.getArrayElement(i);
                string site_id = item.getString("site_id");
                if(site_id == BleSiteID || site_id == TvAdapterSiteID || site_id == ZigbeeSiteID){
                    //从相应的配置站点获取mac
                    string uid;
                    qlibc::QData panelConfigRequest, panelConfigResponse;
                    panelConfigRequest.setString("service_id", "get_self_info");
                    panelConfigRequest.putData("request", qlibc::QData());
                    if(httpUtil::sitePostRequest(ip, 9006, panelConfigRequest, panelConfigResponse)){
                        uid = panelConfigResponse.getData("response").getString("device_id");
                    }
                    if(!uid.empty()){
                        string siteName;
                        siteName.append(uid).append(":").append(site_id);
                        Json::Value siteItem;
                        siteItem["siteName"] = siteName;
                        siteItem["ip"] = ip;
                        siteItem["port"] = item.getInt("port");
                        sitesMap.insert(std::make_pair(siteName, siteItem));
                    }
                }
            }
        }
    }

    //删除已经不存在的站点连接
    SiteRecord::getInstance()->removeSitesNonExist(sitesMap);
    //加入新发现的连接
    for(auto& elem : sitesMap){
        SiteRecord::getInstance()->addSite(elem.first, elem.second["ip"].asString(), elem.second["port"].asInt());
    }

    //订阅蓝牙站点的消息
//    std::set<string> siteNames = SiteRecord::getInstance()->getSiteName();
//    for(auto& siteName : siteNames){
//        smatch sm;
//        if(regex_match(siteName, sm, regex("(.*):ble_light"))){
//            //订阅蓝牙站点消息
//            int code;
//            std::vector<string> messageIdList;
//            messageIdList.push_back(ScanResultMsg);
//            messageIdList.push_back(SingleDeviceBindSuccessMsg);
//            messageIdList.push_back(SingleDeviceUnbindSuccessMsg);
//            messageIdList.push_back(BindEndMsg);
//            messageIdList.push_back(Device_State_Changed);
//            code = ServiceSiteManager::subscribeMessage("127.0.0.1", 9001, messageIdList);
//            if (code == ServiceSiteManager::RET_CODE_OK) {
//                printf("subscribeMessage whiteListModified ok.\n");
//                break;
//            }
//        }
//    }
}

qlibc::QData siteManager::getPanelList(){
    qlibc::QData panelArray;
    qlibc::QData request, response;
    request.setString("service_id", "site_localAreaNetworkSite");
    request.putData("request", qlibc::QData().setString("site_id", ""));
    if(httpUtil::sitePostRequest("127.0.0.1", 9000, request, response)){    //获取局域网内所有发现的站点
        qlibc::QData resBody = response.getData("response");
        Json::Value::Members ipMembers = resBody.getMemberNames();
        for(auto& ip : ipMembers){
            qlibc::QData siteList = resBody.getData(ip);
            Json::ArrayIndex size = siteList.size();
            for(Json::ArrayIndex i = 0; i < size; ++i){
                qlibc::QData item = siteList.getArrayElement(i);
                string site_id = item.getString("site_id");
                if(site_id == BleSiteID || site_id == TvAdapterSiteID || site_id == ZigbeeSiteID){
                    //从相应的配置站点获取mac
                    string uid;
                    qlibc::QData panelConfigRequest, panelConfigResponse;
                    panelConfigRequest.setString("service_id", "get_self_info");
                    panelConfigRequest.putData("request", qlibc::QData());
                    if(httpUtil::sitePostRequest(ip, 9006, panelConfigRequest, panelConfigResponse)){
                        uid = panelConfigResponse.getData("response").getString("device_id");
                    }
                    if(!uid.empty()){
                        Json::Value panelItem;
                        panelItem["uid"] = uid;
                        panelItem["siteId"] = site_id;
                        panelArray.append(panelItem);
                    }
                }
            }
        }
    }
    return panelArray;
}


void siteManager::updateSite1() {
    qlibc::QData request, response;
    request.setString("service_id", "site_localAreaNetworkSite");
    request.putData("request", qlibc::QData().setString("site_id", ""));
    if(httpUtil::sitePostRequest("127.0.0.1", 9000, request, response)){    //获取局域网内所有发现的站点
        qlibc::QData resBody = response.getData("response");
        Json::Value::Members ipMembers = resBody.getMemberNames();
        for(auto& ip : ipMembers){
            qlibc::QData siteList = resBody.getData(ip);
            Json::ArrayIndex size = siteList.size();
            for(Json::ArrayIndex i = 0; i < size; ++i){
                qlibc::QData item = siteList.getArrayElement(i);
                string site_id = item.getString("site_id");
                if(site_id == BleSiteID || site_id == TvAdapterSiteID || site_id == ZigbeeSiteID){
                    //从相应的配置站点获取mac
                    string uid;
                    qlibc::QData panelConfigRequest, panelConfigResponse;
                    panelConfigRequest.setString("service_id", "get_self_info");
                    panelConfigRequest.putData("request", qlibc::QData());
                    if(httpUtil::sitePostRequest(ip, 9006, panelConfigRequest, panelConfigResponse)){
                        uid = panelConfigResponse.getData("response").getString("device_id");
                    }
                    if(!uid.empty()){
                        string siteName;
                        siteName.append(uid).append(":").append(site_id);
                        SiteRecord::getInstance()->addSite(siteName, ip, item.getInt("port"));  //记录选定的站点
                    }
                }
            }
        }
    }
}
