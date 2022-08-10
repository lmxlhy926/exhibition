//
// Created by WJG on 2022-4-26.
//

#ifndef SITEMANAGERSERVICE_SITEMGSERVICE_H
#define SITEMANAGERSERVICE_SITEMGSERVICE_H

#include "qlibc/jsoncpp/json.h"
#include "siteService/service_site_manager.h"


int sitemg_service_start();
int sitemg_service_stop();

void site_ping_run_thread();
int send_http_request(std::string ip, int port, std::string msg, Json::Value &resp);

int start_service_mdns();
int service_mdns();

#endif //SITEMANAGERSERVICE_SITEMGSERVICE_H
