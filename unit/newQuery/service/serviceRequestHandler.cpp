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
    string hostname = "lhy";
    string service = "edgeai.site-query._tcp.local.";
    int service_port = 9000;
    while(service_mdns(hostname.c_str(), service.c_str(), service_port) == -1){
        LOG_RED << "failed to start mdnsService, start again in 3 seconds.....";
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
}

int site_discovery_service_handler(const Request& request, Response& response){

    mdns_query_t query[16];

    query[0].name = "_edgeai.site-query._tcp.local.";
    query[0].length = strlen(query[0].name);
    query[0].type = MDNS_RECORDTYPE_PTR;

    send_mdns_query(query, 1);

    response.set_content(okResponse.dump(), "text/json");
    return 0;
}


