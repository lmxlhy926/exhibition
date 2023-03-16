
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
    std::cout << request.body << std::endl;
}


int main(int argc, char* argv[]) {
    string path = "/data/changhong/edge_midware/lhy/siteScribe.txt";
    muduo::logInitLogger(path);

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
            int code = serviceSiteManager->subscribeMessage("127.0.0.1", 9001, messageIdList);
            if (code == ServiceSiteManager::RET_CODE_OK) {
                printf("subscribeMessage ok.\n");
                break;
            }
            std::this_thread::sleep_for(std::chrono::seconds(3));
            std::cout << "subscribed  failed....., start to subscribe in 3 seconds" << std::endl;
        }
    });


    // 站点监听线程启动
    threadPool_.enqueue([&](){
        while(true){
            //自启动方式
            int code = serviceSiteManager->start();
            if(code != 0){
                std::cout << "===>scribeSite startByRegister error, code = " << code << std::endl;
                std::cout << "===>scribeSite startByRegister in 3 seconds...." << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(3));
            }else{
                std::cout << "===>scribeSite startByRegister successfully....." << std::endl;
                break;
            }
        }
    });

    std::cout << "QUIT1..." << std::endl;
    LOG_RED << "QUIT1...";

    while(true){
        std::this_thread::sleep_for(std::chrono::seconds(60 *10));
    }

    std::cout << "QUIT2..." << std::endl;

    return 0;
}
