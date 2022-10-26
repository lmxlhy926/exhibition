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
#include "http/httplib.h"
#include "qlibc/QData.h"


int sitemg_service_start(){

    start_service_mdns();

    SiteTree &stree = SiteTree::GetInstance();
    stree.initSiteTree("test");

    std::thread sitePingThread(site_ping_run_thread);
    sitePingThread.detach();

    // 创建 serviceSiteManager 对象
    servicesite::ServiceSiteManager *ssm = servicesite::ServiceSiteManager::getInstance();

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
    ssm->registerMessageId(SITE_REGISTERAGAIN_MESSAGE_ID);


    httplib::ThreadPool threadPool_(10);
    threadPool_.enqueue([&](){
        while(true){
            //自启动方式
            int code = ssm->start();

            if(code != 0){
                printf("===>querySite start error, code = %d\n", code);
                printf("===>querySite start in 3 seconds....");
                std::this_thread::sleep_for(std::chrono::seconds(3));
            }else{
                printf("===>querySite start successfully.....");
                break;
            }
        }
    });

    std::this_thread::sleep_for(std::chrono::seconds(1));

    //重新启动后，发布注册消息，使各个站点重新进行注册
    qlibc::QData registerAgain;
    registerAgain.setString("message_id", SITE_REGISTERAGAIN_MESSAGE_ID);
    registerAgain.putData("content", qlibc::QData(Json::Value(Json::objectValue)));
    ssm->publishMessage(SITE_REGISTERAGAIN_MESSAGE_ID, registerAgain.toJsonString());
    printf("===>publish registerAgain message to notify all other sites to register again....\n");


    while(true){
        std::this_thread::sleep_for(std::chrono::seconds(60 *10));
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
