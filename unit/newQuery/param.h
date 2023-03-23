//
// Created by 78472 on 2022/11/17.
//

#ifndef EXHIBITION_PARAM_H
#define EXHIBITION_PARAM_H

#include <string>
using namespace std;

static const string QuerySiteID = "site_query";
static const string QuerySiteName = "服务站点查询";
static const int QuerySitePort = 9000;

//服务ID
static const string Site_Register_Service_ID        = "site_register";                   //站点注册
static const string Site_UnRegister_Service_ID      = "site_unregister";                 //站点注销
static const string Site_Query_Service_ID           = "site_query";                      //站点查询
static const string Site_Ping_Service_ID            = "site_ping";                       //站点ping
static const string Site_LocalSite_Service_ID       = "site_localSite";                  //获取本机站点
static const string Site_LocalAreaSite_Service_ID   = "site_localAreaNetworkSite";       //获取局域网发现的所有站点
static const string Site_LocalAreaSiteExceptOwn_Service_ID = "site_localAreaNetworkSiteExceptLocal";   //获取局域网内，本机外的站点信息

//订阅消息ID
static const string Site_OnOff_Node2Node_MessageID  = "site_OnOff_node2node";       //节点消息通道
static const string Site_OnOffLine_MessageID        = "site_onoffline";             //站点上下线
static const string Site_RegisterAgain_MessageID    = "register2QuerySiteAgain";    //站点重新注册
static const string Site_Requery_Result_MessageID   = "site_query_result";          //站点查询结果

#endif //EXHIBITION_PARAM_H
