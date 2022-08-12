//
// Created by 78472 on 2022/5/18.
//

#ifndef EXHIBITION_PARAM_H
#define EXHIBITION_PARAM_H

static const string RequestIp = "127.0.0.1";

static const int ZigbeeSitePort = 9002;
static const int TvAdapterSitePort = 9008;
static const int SynergySitePort = 9007;
static const int QuerySitePort = 9000;

//站点描述
static const string SYNERGY_SITE_ID = "collaborate";
static const string SYNERGY_SITE_ID_NAME = "协同服务";

//订阅消息
static const string REGISTERAGAIN_MESSAGE_ID = "register2QuerySiteAgain";   //重新向南向站点注册


//服务ID
static const string Control_Device_Service_ID = "deviceControl";



#endif //EXHIBITION_PARAM_H
