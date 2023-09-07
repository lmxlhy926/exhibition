#ifndef NIGHTLIGHT_PARAM_H
#define NIGHTLIGHT_PARAM_H

#include <string>
using namespace std;

static const string QuerySiteID = "site_query";
static const string QuerySiteName = "服务站点查询";
static const int QuerySitePort = 9000;

static const string BleSiteID = "ble_light";
static const string BleSiteName = "BLE灯控";
static const int BleSitePort = 9001;

static const string ZigbeeSiteID = "zigbee_light";
static const string ZigbeeSiteName = "ZigBee灯控";
static const int ZigbeeSitePort = 9002;

static const string SynergySiteID = "collaborate";
static const string SynergySiteName = "协同服务";
static const int SynergySitePort = 9007;

static const string LightFlowSiteID = "light_flow";
static const string LightFlowSiteName = "夜灯跟随";
static const int LightFlowSitePort = 9020;



//服务ID
static const string SaveStrip_Service_ID          = "saveStrip";           //保存灯带
static const string DelStrip_Service_ID           = "delStrip";            //删除灯带
static const string GetStripList_Service_ID       = "getStripList";        //获取灯带列表


//订阅消息ID
static const string Radar_Msg_MessageID   =   "reportAllTargets";


#endif