//
// Created by 78472 on 2022/6/9.
//

#include "messageSubscribeHandler.h"
#include "siteService/service_site_manager.h"
#include "qlibc/QData.h"
#include "param.h"

using namespace servicesite;

void deviceStatus(const Request& request){
    qlibc::QData requestData(request.body);
    ServiceSiteManager* serviceSiteManager = ServiceSiteManager::getInstance();
    serviceSiteManager->publishMessage(DEVICE_STATUS_MESSAGE_ID, requestData.toJsonString());
}