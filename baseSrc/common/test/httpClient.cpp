#include <iostream>
#include "qlibc/QData.h"
#include "common/httplib.h"
#include "log/Logging.h"
#include "../httpUtil.h"


void clientTest(){
    qlibc::QData data;
    data.setString("service_id", "whiteListRequest");
    data.putData("request", qlibc::QData());

    httplib::Client client_("127.0.0.1", 9006);

    while(true){
        auto httpRes = client_.Post("/", data.toJsonString(), "application/json");
        LOG_INFO << ".....post.....";
        if(httpRes != nullptr){
            LOG_YELLOW << httpRes->body;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds (1));
    }
}



int main(int argc, char* argv[]){
    SiteRecord::getInstance()->addSite("config", "127.0.0.1", 9000);
    SiteRecord::getInstance()->addSite("query", "127.0.0.1", 90006);
    SiteRecord::getInstance()->addSite("query", "127.0.0.1", 90007);

    qlibc::QData request;
    request.setString("hello", "world");

    qlibc::QData response;

    bool flag = SiteRecord::getInstance()->sendRequest2Site("config", request, response);
    std::cout << "flag: " << flag << std::endl;

    SiteRecord::getInstance()->printMap();

    return 0;
}


