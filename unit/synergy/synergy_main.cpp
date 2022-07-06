
#include <thread>
#include <unistd.h>
#include <atomic>
#include <cstdio>
#include <functional>

#include "siteService/nlohmann/json.hpp"
#include "siteService/service_site_manager.h"

#include "qlibc/FileUtils.h"
#include "serviceRequestHandler.h"
#include "param.h"

using namespace std;
using namespace servicesite;
using namespace httplib;
using namespace std::placeholders;
using json = nlohmann::json;

static const string SYNERGY_SITE_ID = "collaborate";
static const string SYNERGY_SITE_ID_NAME = "协同服务";


int main(int argc, char* argv[]) {

    httplib::ThreadPool threadPool_(30);
    std::atomic<bool> http_server_thread_end(false);

    // 创建 serviceSiteManager 对象, 单例
    ServiceSiteManager* serviceSiteManager = ServiceSiteManager::getInstance();
    serviceSiteManager->setServerPort(SynergySitePort);
    serviceSiteManager->setSiteIdSummary(SYNERGY_SITE_ID, SYNERGY_SITE_ID_NAME);

    // 注册 Service 请求处理 handler
    serviceSiteManager->registerServiceRequestHandler(Control_Device_Service_ID,
                                                      [&](const Request& request, Response& response) -> int{
        return device_control_service_handler(request, response);
                                                      });


    // 站点监听线程启动
    threadPool_.enqueue([&](){
        while(true){
            //自启动方式
            int code = serviceSiteManager->start();
            //注册启动方式
//            int code = serviceSiteManager->startByRegister();
            if(code != 0){
                std::cout << "===>synergySite startByRegister error, code = " << code << std::endl;
                std::cout << "===>synergySite startByRegister in 3 seconds...." << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(3));
            }else{
                std::cout << "===>configSite startByRegister successfully....." << std::endl;
                break;
            }
        }
    });

    while(true){
        std::this_thread::sleep_for(std::chrono::seconds(60 *10));
    }

    return 0;
}
