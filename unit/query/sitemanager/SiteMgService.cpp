//
// Created by WJG on 2022-4-26.
//

#include "SiteMgService.h"
#include "mdns/mdns.h"
#include "SiteTree.h"
#include "SiteRequestProcess.h"
#include "comm_define.h"
#include "SiteRequestProcess.h"
#include "log_tool.h"
#include "StringUtils.h"
#include "mdns/mdns_interface.h"
#include "common/httplib.h"


//servicesite::ServiceSiteManager *ssm = nullptr;

int sitemg_service_start(){

    start_service_mdns();

    SiteTree &stree = SiteTree::GetInstance();
    stree.initSiteTree("test");

    std::thread sitePingThread(site_ping_run_thread);
    sitePingThread.detach();

    // 创建 serviceSiteManager 对象
    servicesite::ServiceSiteManager *ssm = servicesite::ServiceSiteManager::getInstance();
    if(nullptr != ssm)
    {
        // 设置 site id, summary
        ssm->setSiteIdSummary("site_query", "服务站点查询");

        // 设置 http 服务器端口
        ssm->setServerPort(SITE_QUERY_SERVICE_PORT);

        // 注册 Service 请求处理 handler
        ssm->registerServiceRequestHandler(SITE_REGISTER_SERVICE_ID, site_register);
        ssm->registerServiceRequestHandler(SITE_QUERY_SERVICE_ID, site_query);
        ssm->registerServiceRequestHandler(SITE_PING_SERVICE_ID, site_ping);

        // 注册支持的消息ID， 有两个消息
        ssm->registerMessageId(SITE_ONOFFLINE_MESSAGE_ID);

        // 注册 Message 请求处理 handler
//    ssm->registerMessageHandler(TEST_MESSAGE_ID_1, message_handler_1);

        // 启动服务器
        int ret = ssm->start();

        // 通过注册的方式启动服务器， 需要提供site_id, site_name
        // code = ssm->startByRegister(TEST_SITE_ID_1, TEST_SITE_NAME_1);

        if (ret != 0)
            printf("star error. code = %d\n", ret);
    }

    return 0;
}

int send_http_request(std::string ip, int port, std::string msg, Json::Value &resp){
    httplib::Client cli(ip, port);
    auto res = cli.Post("/", msg, "text/plain");
    if(res)
    {
        if (res->status != 200)
        {
            printf("http status = %d, error.\n", res->status);
            return -1;
        }

        resp = StringUtils::parseJson(res->body);
        return 0;
    }
    else
    {
        printf("http connect error.\n");
        return -1;
    }
}

void site_ping_run_thread(){
    SiteTree &stree = SiteTree::GetInstance();
    std::unordered_set<std::string> site_list;
    std::unordered_set<std::string> remote_querysite_list;
    int ret;
    while (true)
    {
        sleep_s(20);

        site_list = stree.getSiteIdList();
        for(auto item : site_list)
        {
            ret = stree.refreshSitePingCounter(item, 0);
            if(ret <= -3)
            {
                Json::Value site_info = stree.getSiteAllInfo(item);
                site_info[SITE_STATUS] = OFF_LINE;
                std::string msg_str = StringUtils::valueToJsonString(site_info);

                servicesite::ServiceSiteManager *ssm = servicesite::ServiceSiteManager::getInstance();
                if(nullptr != ssm)
                    ssm->publishMessage(SITE_ONOFFLINE_MESSAGE_ID, msg_str);

                stree.notifySiteOffline(item);
            }
        }

//        send_mdns_query(SITE_QUERY_SERVICE_NAME);
//        ret = get_mdns_query_result(remote_querysite_list);
//        if(ret > 0)
//        {
//            Json::Value req_value, r_obj;
//            r_obj["yourself"] = "yes";
//            req_value[SERVICE_ID] = SITE_QUERY_SERVICE_ID;
//            req_value[REQUEST_KEY] = r_obj;
//            std::string req_str = StringUtils::valueToJsonString(req_value);
//
//            for(auto item : remote_querysite_list)
//            {
//                cout << "mdns_query site :" << item << endl;
//                Json::Value value;
//                int ret = send_http_request(item, SITE_QUERY_SERVICE_PORT, req_str, value);
//                if(ret == 0)
//                {
//                    if(!value.empty())
//                    {
//                        int code = value.get(RESP_CODE, CODE_FAIL).asInt();
//                        if(code == CODE_SUCCESS)
//                        {
//                            Json::Value site_list = value.get(RESPONSE_KEY, Json::nullValue);
//                            stree.setRemoteStie(item, site_list);
//                        }
//                    }
//                }
//            }
//        }

        ret = 0;
        site_list.clear();
        remote_querysite_list.clear();
    }
}
