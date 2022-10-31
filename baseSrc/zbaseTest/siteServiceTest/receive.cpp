

#include "common/httpUtil.h"
#include "siteService/service_site_manager.h"
#include "log/Logging.h"
using namespace servicesite;
using namespace httplib;
using namespace std::placeholders;

string SendSiteName = "send";
string ReceiveSiteName = "receive";
int SendSitePort = 60005;
int ReceiveSitePort = 60006;
string LocalIp = "127.0.0.1";


int main(int argc, char* argv[]){

    int i = 1;
    // 创建 serviceSiteManager 对象, 单例
    ServiceSiteManager* serviceSiteManager = ServiceSiteManager::getInstance();
    serviceSiteManager->setServerPort(ReceiveSitePort);
    serviceSiteManager->setSiteIdSummary("", "");

    httplib::ThreadPool threadPool_(10);

    //注册设备扫描回调
    serviceSiteManager->registerMessageHandler("test",
                                                      [&](const Request& request) -> int{
        if(i == 1 || i == 1000){
            LOG_INFO << qlibc::QData(request.body).toJsonString();
        }
        ++i;

    });

    while(true){
        int code;
        std::vector<string> messageIdList;
        messageIdList.push_back("test");
        code = serviceSiteManager->subscribeMessage(LocalIp, SendSitePort, messageIdList);

        if (code == ServiceSiteManager::RET_CODE_OK) {
            printf("subscribeMessage test ok.\n");
            break;
        }

        std::this_thread::sleep_for(std::chrono::seconds(3));
        printf("subscribed test failed....., start to subscribe in 3 seconds\n");
    }

    threadPool_.enqueue([&]{
        while(true){
            //自启动方式
            int code = serviceSiteManager->start();
            if(code != 0){
                LOG_RED << "===>recevie startByRegister in 3 seconds....";
                std::this_thread::sleep_for(std::chrono::seconds(3));
            }else{
                LOG_INFO << "===>recevie startByRegister successfully.....";
                break;
            }
        }
    });

    while(true){
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }

    return 0;
}