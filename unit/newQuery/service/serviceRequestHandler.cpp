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


void mdnsServiceStart(){
    string hostname = "smartHome";
    char hostname_buffer[256];
    size_t hostname_size = sizeof(hostname_buffer);
    if (gethostname(hostname_buffer, hostname_size) == 0)
        hostname = hostname_buffer;

    string service = "edgeai.site-query._tcp.local.";
    int service_port = 45544;
    while(service_mdns(hostname.c_str(), service.c_str(), service_port) == -1){
        LOG_RED << "failed to start mdnsService, start again in 3 seconds.....";
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
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


//站点查询
int site_query_service_handler(const Request& request, Response& response){
    qlibc::QData reqData(request.body);
    LOG_INFO << "site_query_service_handler: " << reqData.toJsonString();
    string site_id = reqData.getData("request").getString("site_id");

    //如果本机存在，则发布
    if(SiteTree::getInstance()->isSiteExist(site_id)){
        qlibc::QData siteInfo = SiteTree::getInstance()->getSiteInfo(site_id);
        qlibc::QData content, publishData;
        content.setString("site_id", site_id);
        content.setString("ip", siteInfo.getString("ip"));
        content.setInt("port", siteInfo.getInt("port"));
        publishData.setString("message_id", "site_query_result");
        publishData.putData("content", content);
        ServiceSiteManager::getInstance()->publishMessage(Site_Requery_Result_MessageID, publishData.toJsonString());
    }

    //查询其它面板站点消息
    string siteMdnsName = "_edgeai." + site_id + "._tcp." + "local.";
    mdns_query_t query[1];
    query[0].name = siteMdnsName.c_str();
    query[0].length = strlen(query[0].name);
    query[0].type = MDNS_RECORDTYPE_PTR;
    send_mdns_query(query, 1);

    response.set_content(okResponse.dump(), "text/json");
    return 0;
}

int site_ping_service_handler(const Request& request, Response& response){
    qlibc::QData reqData(request.body);
    LOG_INFO  << "site_ping_service_handler: " << reqData.toJsonString();
    string site_id = reqData.getData("request").getString("site_id");
    SiteTree::getInstance()->updateSitePingCounter(site_id);
    response.set_content(okResponse.dump(), "text/json");
    return 0;
}