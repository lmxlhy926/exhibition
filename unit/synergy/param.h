//
// Created by 78472 on 2022/5/18.
//

#ifndef EXHIBITION_PARAM_H
#define EXHIBITION_PARAM_H

static const string RequestIp = "127.0.0.1";

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
static const string Control_Device_Service_ID = "deviceControl";
static const string GetDeviceList_Service_ID = "get_device_list";
static const string GetGroupList_Service_ID = "get_group_list";
static const string SceneCommand_Service_ID = "sceneCommand";


//订阅消息ID
static const string DeviceList_changed_messageID = "deviceList_changed";


#endif //EXHIBITION_PARAM_H
