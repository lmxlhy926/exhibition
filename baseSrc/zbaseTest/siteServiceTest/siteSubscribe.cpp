
#include <thread>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#include "common/httpUtil.h"
#include "siteService/nlohmann/json.hpp"
#include "siteService/service_site_manager.h"
#include "log/Logging.h"
#include <map>

using namespace std;
using namespace servicesite;
using namespace httplib;
using json = nlohmann::json;


string time2HourMinute(time_t time){
    struct tm *tm_time = localtime(&time);
    int hour = tm_time->tm_hour;
    int minute = tm_time->tm_min;
    int seconds = tm_time->tm_sec;
    return string().append(std::to_string(hour)).append(":").append(std::to_string(minute)).append(":").append(std::to_string(seconds));
}

void storeData(std::map<time_t, Json::Value>& dataMap){
    qlibc::QData data;
    for(auto& elem : dataMap){
        Json::Value value;
        value[time2HourMinute(elem.first)] = elem.second;
        data.append(value);
    }
    data.saveToFile("./pointData.json", true);
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
    std::map<time_t, Json::Value> dataMap;
    time_t latestMinute = time(nullptr);
    bool start = true;
    for(auto& elem : messageIdList){
        serviceSiteManager->registerMessageId(elem);
        serviceSiteManager->registerMessageHandler(elem, [&serviceSiteManager, &dataMap, &latestMinute, &start](const Request& request){
            qlibc::QData data(request.body);
            time_t now = time(nullptr);
            if(now - latestMinute > 10 || start){
                latestMinute = now;
                Json::Value value;
                value.append(data.asValue());
                dataMap.insert(std::make_pair(now, value));
                start = false;
            }else{
                auto pos = dataMap.find(latestMinute);
                if(pos != dataMap.end()){
                    pos->second.append(data.asValue());
                }
            }
            LOG_PURPLE << "------radarMessage-------";
            string messageId = data.getString("message_id");
            serviceSiteManager->publishMessage(messageId, data.toJsonString());
            
        });
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


    while(true){
        int code = serviceSiteManager->start();
        if(code != 0){
            printf("===>scribeSite startByRegister error....\n");
            printf("===>scribeSite startByRegister in 3 seconds....\n");
            std::this_thread::sleep_for(std::chrono::seconds(3));
        }
    }

    return 0;
}
