
#include <thread>
#include <atomic>
#include <cstdio>

#include "siteService/nlohmann/json.hpp"
#include "siteService/service_site_manager.h"

#include "socket/httplib.h"
#include "serviceRequestHandler.h"
#include "qlibc/FileUtils.h"
#include "paramconfig.h"

using namespace std;
using namespace servicesite;
using namespace httplib;
using namespace std::placeholders;
using json = nlohmann::json;

static const string CONFIG_SITE_ID = "config";
static const string CONFIG_SITE_ID_NAME = "整体配置";


int main(int argc, char* argv[]) {

    //. 设置线程池
    httplib::ThreadPool threadPool_(30);
    std::atomic<bool> http_server_thread_end(false);

    //. 配置本站点启动信息
    ServiceSiteManager* serviceSiteManager = ServiceSiteManager::getInstance();
    serviceSiteManager->setServerPort(ConfigSitePort);
    serviceSiteManager->setSiteIdSummary(CONFIG_SITE_ID, CONFIG_SITE_ID_NAME);

    //. 设置配置文件加载路径
    configParamUtil* configPathPtr = configParamUtil::getInstance();
    configPathPtr->setConfigPath(string(argv[1]));

    //注册请求场景列表处理函数
    serviceSiteManager->registerServiceRequestHandler(WHITELIST_REQUEST_SERVICE_ID, whiteList_service_get_handler);
    //注册子设备注册处理函数
    serviceSiteManager->registerServiceRequestHandler(WHITELIST_PUT_SERVICE_ID, whiteList_service_put_handler);


    // 站点监听线程启动
    threadPool_.enqueue([&](){
        // 启动服务器，参数为端口， 可用于单独的开发调试
        int code = serviceSiteManager->start();

        // 通过注册的方式启动服务器， 需要提供site_id, site_name, port
//    	int code = serviceSiteManager->startByRegister();

        if (code != 0) {
            printf("start error. code = %d\n", code);
        }

        http_server_thread_end.store(true);
    });

    std::this_thread::sleep_for(std::chrono::seconds(5));

    while(true){
        if (http_server_thread_end.load()){
            printf("http end abnormally....\n");
            break;
        }
        std::this_thread::sleep_for(std::chrono::seconds(30));
    }

    return -1;
}
