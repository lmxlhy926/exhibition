//
// Created by 78472 on 2022/11/17.
//

#ifndef EXHIBITION_SERVICEREQUESTHANDLER_H
#define EXHIBITION_SERVICEREQUESTHANDLER_H

#include <string>
#include <http/httplib.h>
using namespace httplib;

//开启mdns服务
void mdnsServiceStart();

//站点注册
int site_register_service_handler(const Request& request, Response& response);

//站点注销
int site_unRegister_service_handler(const Request& request, Response& response);

//站点查询
int site_query_service_handler(const Request& request, Response& response, httplib::ThreadPool& threadPool);

//站点心跳
int site_ping_service_handler(const Request& request, Response& response);

//获取节点站点信息
int site_getAllLocalSiteInfo_service_handler(const Request& request, Response& response);



#endif //EXHIBITION_SERVICEREQUESTHANDLER_H
