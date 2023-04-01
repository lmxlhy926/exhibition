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

siteManager* siteManager::Instance = nullptr;

siteManager *siteManager::getInstance() {
    if(Instance == nullptr){
        Instance = new siteManager();
    }
    return Instance;
}

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
                if(site_id == BleSiteID || site_id == TvAdapterSiteID || site_id == ZigbeeSiteID){    //只关心设备类站点
                    //从相应的配置站点获取mac
                    string uid;
                    qlibc::QData panelConfigRequest, panelConfigResponse;
                    panelConfigRequest.setString("service_id", "get_self_info");
                    panelConfigRequest.putData("request", qlibc::QData());
                    if(httpUtil::sitePostRequest(ip, 9006, panelConfigRequest, panelConfigResponse)){   //获取面板配置信息
                        uid = panelConfigResponse.getData("response").getString("device_id");
                    }
                    if(!uid.empty()){   //构造站点信息
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

    //打印发现的设备站点
    std::set<string> siteNames = SiteRecord::getInstance()->getSiteName();
    for(auto& siteName : siteNames){
        string ip;
        int port;
        SiteRecord::getInstance()->getSiteInfo(siteName, ip, port);
        LOG_PURPLE << "ExistSite: " << siteName << ": <" << ip << ", " << port << ">...";
    }

    //订阅蓝牙站点的消息
    for(auto& siteName : siteNames){
        smatch sm;
        if(regex_match(siteName, sm, regex("(.*):ble_light"))){
            //获取ip，端口号
            string ip;
            int port;
            if(SiteRecord::getInstance()->getSiteInfo(siteName, ip, port)){
                //订阅蓝牙站点消息
                std::vector<string> messageIdList;
                messageIdList.push_back(ScanResultMsg);
                messageIdList.push_back(SingleDeviceBindSuccessMsg);
                messageIdList.push_back(SingleDeviceUnbindSuccessMsg);
                messageIdList.push_back(BindEndMsg);
                messageIdList.push_back(Device_State_Changed);
                ServiceSiteManager::subscribeMessage(ip, port, messageIdList);
            }
        }
    }
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
                    LOG_GREEN << "getPanelInfoRequest: " << panelConfigRequest.toJsonString();
                    if(httpUtil::sitePostRequest(ip, 9006, panelConfigRequest, panelConfigResponse)){
                        LOG_BLUE << "getPanelInfoResponse: " << panelConfigResponse.toJsonString();
                        qlibc::QData panelData = panelConfigResponse.getData("response");
                        if(!panelData.empty()){
                            panelData.setString("siteId", site_id);
                            panelArray.append(panelData);
                        }
                    }
                }
            }
        }
    }
    return panelArray;
}

