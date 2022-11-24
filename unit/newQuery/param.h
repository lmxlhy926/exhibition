//
// Created by 78472 on 2022/11/17.
//

#ifndef EXHIBITION_PARAM_H
#define EXHIBITION_PARAM_H

#include <string>
using namespace std;

static const string BleSiteID = "ble_light";
static const string BleSiteName = "BLE灯控";
static const int BleSitePort = 9001;

static const string ZigbeeSiteID = "zigbee_light";
static const string ZigbeeSiteName = "ZigBee灯控";
static const int ZigbeeSitePort = 9002;

static const string TvAdapterSiteID = "tv_adapter";
static const string TvAdapterSiteName = "智汇家adapter";
static const int TvAdapterSitePort = 9008;

static const string SynergySiteID = "collaborate";
static const string SynergySiteName = "协同服务";
static const int SynergySitePort = 9007;

static const string SceneSiteID = "scene";
static const string SceneSiteName = "场景";
static const int SceneSitePort = 9011;

static const string QuerySiteID = "site_query";
static const string QuerySiteName = "服务站点查询";
static const int QuerySitePort = 9000;


//服务ID
static const string Site_Register_Service_ID = "site_register";             //站点注册
static const string Site_UnRegister_Service_ID = "site_unregister";         //站点注销
static const string Site_Query_Service_ID = "site_query";                   //站点查询
static const string Site_Ping_Service_ID = "site_ping";                     //站点ping
static const string Site_localAllSite_Service_ID = "site_localAllSite";     //获取本机所有站点信息

//订阅消息ID
static const string Node2Node_MessageID = "node2node";                           //节点消息通道
static const string Site_OnOffLine_MessageID = "site_onoffline";                 //站点上下线
static const string Site_RegisterAgain_MessageID = "register2QuerySiteAgain";    //站点重新注册
static const string Site_Requery_Result_MessageID = "site_query_result";         //站点查询结果

#endif //EXHIBITION_PARAM_H
