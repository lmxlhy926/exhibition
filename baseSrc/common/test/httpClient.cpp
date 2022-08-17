#include <iostream>
#include "qlibc/QData.h"
#include "common/httplib.h"
#include "log/Logging.h"

//{
//"service_id": "sceneListRequest",
//"request": {}
//}

int main(int argc, char* argv[]){

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


    return 0;
}


