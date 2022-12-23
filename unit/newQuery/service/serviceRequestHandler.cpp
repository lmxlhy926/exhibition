//
// Created by 78472 on 2022/11/17.
//

#include "serviceRequestHandler.h"
#include <thread>
#include "param.h"
#include "qlibc/QData.h"
#include "log/Logging.h"
#include "mdns/mdnsUtil.h"
#include "siteService/nlohmann/json.hpp"
#include "siteService/service_site_manager.h"
#include "siteManage/siteManageUtil.h"

using namespace servicesite;

static const nlohmann::json okResponse = {
        {"code",     0},
        {"error",    "ok"},
        {"response", {}}
};

static const nlohmann::json errResponse = {
        {"code",     1},
        {"error",    "failed"},
        {"response", {}}
};

qlibc::QData successResponse(qlibc::QData& resData){
    qlibc::QData data;
    data.setInt("code", 0);
    data.setString("error", "ok");
    data.putData("response", resData);
    return data;
}

void mdnsServiceStart(){
    string hostname = "smartHome";
    char hostname_buffer[256];
    size_t hostname_size = sizeof(hostname_buffer);
    if (gethostname(hostname_buffer, hostname_size) == 0)
        hostname = hostname_buffer;

    //mdns服务器监听5353端口号，此处指定请求服务名以及返回的端口号
    string service = "edgeai.site-query._tcp.local.";
    int service_port = 9000;
    while(true){
        service_mdns(hostname.c_str(), service.c_str(), service_port);
        LOG_RED << "failed to start mdnsService, start again in 3 seconds.....";
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
}

void site_query_node2node_message_handler(const Request& request){
    qlibc::QData reqData(request.body);
    LOG_INFO << "Received node2node_message: " << reqData.toJsonString();
    reqData.setString("message_id", Site_OnOffLine_MessageID);
    qlibc::QData content = reqData.getData("content");
    //更新发现节点下挂的站点信息
    SiteTree::getInstance()->updateFindSite(content);
    //发布其它节点站点的上下线消息
    ServiceSiteManager::getInstance()->publishMessage(Site_OnOffLine_MessageID, reqData.toJsonString());
    LOG_INFO << "Publish onoffline: " << reqData.toJsonString();
}

//站点注册
int site_register_service_handler(const Request& request, Response& response){
    qlibc::QData reqData(request.body);
    LOG_INFO << "site_register_service_handler: " << reqData.toJsonString();

    string site_id = reqData.getData("request").getString("site_id");
    int port = reqData.getData("request").getInt("port");
    string summary = reqData.getData("request").getString("summary");

    qlibc::QData registerData;
    registerData.setString("ip", SiteTree::getInstance()->getLocalIpAddress());
    registerData.setInt("port", port);
    registerData.setString("site_id", site_id);
    registerData.setString("summary", summary);
    registerData.setString("site_status", "online");

    SiteTree::getInstance()->siteRegister(site_id, registerData.asValue());

    response.set_content(okResponse.dump(), "text/json");
    return 0;
}


//站点注销
int site_unRegister_service_handler(const Request& request, Response& response){
    qlibc::QData reqData(request.body);
    LOG_INFO << "site_unRegister_service_handler: " << reqData.toJsonString();
    string site_id = reqData.getData("request").getString("site_id");
    SiteTree::getInstance()->siteUnregister(site_id);
    response.set_content(okResponse.dump(), "text/json");
    return 0;
}


//进行一次节点发现扫描
int site_query_service_handler(const Request& request, Response& response, httplib::ThreadPool& threadPool){
    qlibc::QData reqData(request.body);
    LOG_INFO << "site_query_service_handler: " << reqData.toJsonString();
    string site_id = reqData.getData("request").getString("site_id");

    //查询其它面板站点消息
    threadPool.enqueue([site_id]{
        string siteMdnsName = "_edgeai." + site_id + "._tcp." + "local.";
        mdns_query_t query[1];
        query[0].name = siteMdnsName.c_str();
        query[0].length = strlen(query[0].name);
        query[0].type = MDNS_RECORDTYPE_PTR;
        send_mdns_query(query, 1);
    });

    response.set_content(okResponse.dump(), "text/json");
    return 0;
}

int site_ping_service_handler(const Request& request, Response& response){
    qlibc::QData reqData(request.body);
    LOG_INFO  << "site_ping_service_handler: " << reqData.toJsonString();
    string site_id = reqData.getData("request").getString("site_id");
    SiteTree::getInstance()->updateLocalSitePingCounter(site_id);
    response.set_content(okResponse.dump(), "text/json");
    return 0;
}

int site_getLocalSiteInfo_service_handler(const Request& request, Response& response){
    qlibc::QData reqData(request.body);
    LOG_INFO  << "site_getLocalSiteInfo_service_handler: " << reqData.toJsonString();
    string site_id = reqData.getData("request").getString("site_id");
    qlibc::QData siteList;
    siteList.putData("siteList", SiteTree::getInstance()->getLocalSiteInfo(site_id));
    response.set_content(successResponse(siteList).toJsonString(), "text/json");
    return 0;
}

int site_getLocalAreaNetworkSiteInfo_service_handler(const Request& request, Response& response){
    qlibc::QData reqData(request.body);
    LOG_INFO  << "site_getLocalAreaNetworkSiteInfo_service_handler: " << reqData.toJsonString();
    string site_id = reqData.getData("request").getString("site_id");
    qlibc::QData siteList = SiteTree::getInstance()->getLocalAreaSite(site_id);
    response.set_content(successResponse(siteList).toJsonString(), "text/json");
    return 0;
}

int printResource(const Request& request, Response& response){
    qlibc::QData data = SiteTree::getInstance()->printResource();
    response.set_content(data.toJsonString(), "text/json");
    return 0;
}