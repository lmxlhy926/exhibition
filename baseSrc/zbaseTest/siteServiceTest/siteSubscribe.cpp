
#include <thread>
#include "common/httpUtil.h"
#include "siteService/nlohmann/json.hpp"
#include "siteService/service_site_manager.h"

using namespace std;
using namespace servicesite;
using namespace httplib;
using json = nlohmann::json;


void messageHandler(const Request& request){
    qlibc::QData data(request.body);
    printf("%s\n", data.toJsonString().c_str());
    qlibc::QData response;
    httpUtil::sitePostRequest("172.29.90.147", 9007, data, response);
}

int main(int argc, char* argv[]) {
    qlibc::QData configData;
    configData.loadFromFile("./config.json");
    string ip = configData.getString("ip");
    int port = configData.getInt("port");
    qlibc::QData messageIdData = configData.getData("messageIds");
    printf("ip: %s\n", ip.c_str());
    printf("port: %d\n", port);
    printf("messageIds: %s\n", messageIdData.toJsonString().c_str());
    Json::ArrayIndex size = messageIdData.size();
    std::vector<string> messageIdList;
    for(int i = 0; i < size; ++i){
        qlibc::QData ithData = messageIdData.getArrayElement(i);
        messageIdList.push_back(ithData.asValue().asString());
    }

    httplib::ThreadPool threadPool_(10);
    // 创建 serviceSiteManager 对象, 单例
    ServiceSiteManager* serviceSiteManager = ServiceSiteManager::getInstance();
    serviceSiteManager->setServerPort(8999);
    ServiceSiteManager::setSiteIdSummary("scribeSite", "订阅测试站点");

    //注册白名单改变处理函数
    for(auto& elem : messageIdList){
        serviceSiteManager->registerMessageHandler(elem, messageHandler);
    }

   
    threadPool_.enqueue([&](){
        while(true){
            int code = serviceSiteManager->subscribeMessage(ip, port, messageIdList);
            if (code == ServiceSiteManager::RET_CODE_OK) {
                printf("subscribeMessage ok...\n");
                break;
            }
            std::this_thread::sleep_for(std::chrono::seconds(3));
            printf("subscribed  failed....., start to subscribe in 3 seconds...\n");
        }
    });


    // 站点监听线程启动
    threadPool_.enqueue([&](){
        while(true){
            //自启动方式
            int code = serviceSiteManager->start();
            if(code != 0){
                printf("===>scribeSite startByRegister error....\n");
                printf("===>scribeSite startByRegister in 3 seconds....\n");
                std::this_thread::sleep_for(std::chrono::seconds(3));
            }
        }
    });


    while(true){
        std::this_thread::sleep_for(std::chrono::seconds(60 *10));
    }

    return 0;
}
