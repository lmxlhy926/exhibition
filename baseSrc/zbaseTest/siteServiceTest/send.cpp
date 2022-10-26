

#include "common/httpUtil.h"
#include "siteService/service_site_manager.h"
#include "log/Logging.h"
#include "log/TimeStamp.h"
#include "qlibc/QData.h"
using namespace servicesite;
using namespace httplib;
using namespace std::placeholders;

string SendSiteName = "send";
string ReceiveSiteName = "receive";
int SendSitePort = 60005;
int ReceiveSitePort = 60006;
string LocalIp = "127.0.0.1";


int main(int argc, char* argv[]){

    // 创建 serviceSiteManager 对象, 单例
    ServiceSiteManager* serviceSiteManager = ServiceSiteManager::getInstance();
    serviceSiteManager->setServerPort(SendSitePort);
    serviceSiteManager->setSiteIdSummary("", "");

    httplib::ThreadPool threadPool_(10);

    serviceSiteManager->registerMessageId("test");  //扫描结果

    threadPool_.enqueue([&]{
        while(true){
            //自启动方式
            int code = serviceSiteManager->start();
            if(code != 0){
                LOG_RED << "===>send startByRegister in 3 seconds....";
                std::this_thread::sleep_for(std::chrono::seconds(3));
            }else{
                LOG_INFO << "===>send startByRegister successfully.....";
                break;
            }
        }
    });

    this_thread::sleep_for(std::chrono::seconds(2));

    qlibc::QData data;
//    data.setString("message_id", "test");
//    data.putData("content", qlibc::QData().setString("key", "world"));
    data.loadFromFile(R"(D:\bywg\project\exhibition\baseSrc\siteService\test\radar.json)");


    std::vector<string> vec;
    for(int i = 0; i < 10000; ++i){
        serviceSiteManager->publishMessage("test", data.toJsonString());
        if(i == 0 || i == 9999){
            LOG_INFO << muduo::TimeStamp::toFormattedString();
        }
    }

//    qlibc::QData saveData;
//    for(auto& elem : vec){
//        saveData.append(elem);
//    }
//    saveData.saveToFile("D:\\bywg\\project\\exhibition\\baseSrc\\siteService\\test\\sendSave.json", true);
//    LOG_RED << "FINISH.........";

    while(true){
        this_thread::sleep_for(std::chrono::seconds(10));
    }

    return 0;
}