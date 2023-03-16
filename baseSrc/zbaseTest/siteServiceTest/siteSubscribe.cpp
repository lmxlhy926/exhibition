
#include <thread>
#include "common/httpUtil.h"
#include "siteService/nlohmann/json.hpp"
#include "siteService/service_site_manager.h"
#include "log/Logging.h"

using namespace std;
using namespace servicesite;
using namespace httplib;
using json = nlohmann::json;

void messageHandler(const Request& request){
    LOG_INFO << request.body;
}


int main(int argc, char* argv[]) {
//    string path = "/data/changhong/edge_midware/lhy/siteScribe.txt";
//    muduo::logInitLogger(path);

    httplib::ThreadPool threadPool_(10);
    // 创建 serviceSiteManager 对象, 单例
    ServiceSiteManager* serviceSiteManager = ServiceSiteManager::getInstance();
    serviceSiteManager->setServerPort(8999);
    ServiceSiteManager::setSiteIdSummary("scribeSite", "订阅测试站点");

    //注册白名单改变处理函数
    serviceSiteManager->registerMessageHandler( "scanResultMsg", messageHandler);
    serviceSiteManager->registerMessageHandler( "singleDeviceBindSuccessMsg", messageHandler);
    serviceSiteManager->registerMessageHandler( "bindEndMsg", messageHandler);
    serviceSiteManager->registerMessageHandler( "singleDeviceUnbindSuccessMsg", messageHandler);
    serviceSiteManager->registerMessageHandler( "device_state_changed", messageHandler);

    threadPool_.enqueue([&](){
        while(true){
            std::vector<string> messageIdList{
                    "scanResultMsg",
                    "singleDeviceBindSuccessMsg",
                    "bindEndMsg",
                    "singleDeviceUnbindSuccessMsg",
                    "device_state_changed"
            };
            int code = serviceSiteManager->subscribeMessage("192.168.137.55", 9007, messageIdList);
            if (code == ServiceSiteManager::RET_CODE_OK) {
                LOG_INFO << "subscribeMessage ok.";
                break;
            }
            std::this_thread::sleep_for(std::chrono::seconds(3));
            LOG_INFO << "subscribed  failed....., start to subscribe in 3 seconds";
        }
    });


    // 站点监听线程启动
    threadPool_.enqueue([&](){
        while(true){
            //自启动方式
            int code = serviceSiteManager->start();
            if(code != 0){
                LOG_INFO << "===>scribeSite startByRegister error, code = ";
                LOG_INFO << "===>scribeSite startByRegister in 3 seconds....";
                std::this_thread::sleep_for(std::chrono::seconds(3));
            }else{
                LOG_INFO << "===>scribeSite startByRegister successfully.....";
                break;
            }
        }
    });


    while(true){
        std::this_thread::sleep_for(std::chrono::seconds(60 *10));
    }

    return 0;
}
