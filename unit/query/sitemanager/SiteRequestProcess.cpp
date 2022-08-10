//
// Created by WJG on 2022-4-26.
//

#include <unordered_map>
#include "SiteRequestProcess.h"
#include "comm_define.h"
#include "SiteTree.h"
#include "StringUtils.h"
#include "SiteMgService.h"


int site_register(const httplib::Request &req, httplib::Response &res){
    Json::Value req_body = StringUtils::parseJson(req.body);
    Json::Value cmd_param = req_body.get(REQUEST_KEY, Json::nullValue);

    if(!cmd_param.isObject())
    {
        Json::Value err_rsp;
        err_rsp[RESP_CODE] = CODE_PARAM_ERR;
        err_rsp[MESSAGE] = RESPON_FAIL_MSG;
        std::string jstr = StringUtils::valueToJsonString(err_rsp);
        res.set_content(jstr, "text/plain");
        res.status = 200;
        return 0;
    }
    std::string site_id = cmd_param.get(SITE_ID, "").asString();
    int site_port = cmd_param.get(SITE_PORT, 0).asInt();
    if(site_id.empty()||(site_port<9000))
    {
        Json::Value err_rsp;
        err_rsp[RESP_CODE] = CODE_PARAM_ERR;
        err_rsp[MESSAGE] = RESPON_FAIL_MSG;
        std::string jstr = StringUtils::valueToJsonString(err_rsp);
        res.set_content(jstr, "text/plain");
        res.status = 200;
        return 0;
    }

    std::string from_ip = req.remote_addr;
    cmd_param[SITE_IP] = from_ip;
    cmd_param[SITE_STATUS] = ON_LINE;

    SiteTree &stree = SiteTree::GetInstance();
    stree.cacheSiteInfo(site_id, cmd_param);
    stree.refreshSitePingCounter(site_id, 1);

    Json::Value resp;
    resp[RESP_CODE] = CODE_SUCCESS;
    resp[MESSAGE] = MSG_SUCCESS;
    std::string jstr = StringUtils::valueToJsonString(resp);
    res.set_content(jstr, "text/plain");
    res.status = 200;

    Json::Value msg;
    msg[MESSAGE_ID] = SITE_ONOFFLINE_MESSAGE_ID;
    msg[CONTENT_KEY] = cmd_param;
    std::string msg_str = StringUtils::valueToJsonString(msg);
    servicesite::ServiceSiteManager *ssm = servicesite::ServiceSiteManager::getInstance();
    if(nullptr != ssm)
        ssm->publishMessage(SITE_ONOFFLINE_MESSAGE_ID, msg_str);

    return 0;
}

int site_query(const httplib::Request &req, httplib::Response &res){
    Json::Value req_body = StringUtils::parseJson(req.body);
    Json::Value cmd_param = req_body.get(REQUEST_KEY, Json::nullValue);
    SiteTree &stree = SiteTree::GetInstance();
    Json::Value value, tmp_v;
    std::string site_id;

    Json::Value query_id;
    if(cmd_param.isObject())
        query_id = cmd_param.get(SITE_ID, Json::nullValue);

    if(query_id.empty())
        value = stree.getAllSite();

    else if(query_id.isString())
    {
        site_id = query_id.asString();
        tmp_v = stree.getSiteAllInfo(site_id);
        if(!tmp_v.empty())
            value.append(tmp_v);
    }

    else if(query_id.isArray())
    {
        int cnt = query_id.size();
        for(int i=0; i<cnt; i++)
        {
            site_id = query_id[i].asString();
            tmp_v = stree.getSiteAllInfo(site_id);
            if(!tmp_v.empty())
                value.append(tmp_v);
        }
    }

    Json::Value resp, data;
    if(!value.empty())
        data[SITE_LIST] = value;

    resp[RESP_CODE] = CODE_SUCCESS;
    resp[MESSAGE] = MSG_SUCCESS;
    resp[RESPONSE_KEY] = data;
    std::string jstr = StringUtils::valueToJsonString(resp);
    res.set_content(jstr, "text/plain");
    res.status = 200;

    return 0;
}

int site_ping(const httplib::Request &req, httplib::Response &res){
    Json::Value req_body = StringUtils::parseJson(req.body);
    Json::Value cmd_param = req_body.get(REQUEST_KEY, Json::nullValue);

    if(cmd_param.isObject())
    {
        std::string site_id = cmd_param.get(SITE_ID, "").asString();
        if(!site_id.empty())
        {
            SiteTree &stree = SiteTree::GetInstance();
            stree.refreshSitePingCounter(site_id, 1);
        }
    }

    Json::Value resp, data;
    data[SITE_ID] = MY_SITE_ID_NAME;
    resp[RESP_CODE] = CODE_SUCCESS;
    resp[MESSAGE] = MSG_SUCCESS;
    resp[RESPONSE_KEY] = data;
    std::string jstr = StringUtils::valueToJsonString(resp);
    res.set_content(jstr, "text/plain");
    res.status = 200;

    return 0;
}
