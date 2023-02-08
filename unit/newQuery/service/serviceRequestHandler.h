//
// Created by 78472 on 2022/11/17.
//

#ifndef EXHIBITION_SERVICEREQUESTHANDLER_H
#define EXHIBITION_SERVICEREQUESTHANDLER_H

#include <string>
#include <http/httplib.h>
using namespace httplib;


//站点注册服务
int site_register_service_handler(const Request& request, Response& response);

//站点注销服务
int site_unRegister_service_handler(const Request& request, Response& response);

//站点查询服务
int site_query_service_handler(const Request& request, Response& response, httplib::ThreadPool& threadPool);

//站点心跳服务
int site_ping_service_handler(const Request& request, Response& response);

//获取本机站点信息服务
int site_getLocalSiteInfo_service_handler(const Request& request, Response& response);

//获取局域网内所有主机下的站点信息服务
int site_getLocalAreaNetworkSiteInfo_service_handler(const Request& request, Response& response);

//查询节点之间的消息处理函数
void site_query_node2node_message_handler(const Request& request);

//打印资源信息
int printIpAddress(const Request& request, Response& response);

#endif //EXHIBITION_SERVICEREQUESTHANDLER_H
