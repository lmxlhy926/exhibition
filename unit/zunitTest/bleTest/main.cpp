
#include <iostream>
#include "common/httpUtil.h"
#include "qlibc/QData.h"

void test(){
    string bleSiteName = "ble_light";
    string LocalIp = "192.168.58.116";
    int bleSitePort = 9001;

    SiteRecord::getInstance()->addSite(bleSiteName, LocalIp, bleSitePort);

    qlibc::QData openList, closeList, response;
    openList.loadFromFile(R"(D:\bywg\project\exhibition\unit\bleTest\open.json)");
    closeList.loadFromFile(R"(D:\bywg\project\exhibition\unit\bleTest\close.json)");

    for(int index = 0; index < 5; ++index){
        //依次打开客厅几个组的灯
        Json::ArrayIndex openListSize = openList.size();
        for(Json::ArrayIndex i = 0; i < openListSize; ++i){
            qlibc::QData controlData = openList.getArrayElement(i);
            SiteRecord::getInstance()->sendRequest2Site(bleSiteName, controlData, response);
            std::cout << "open: " << i << std::endl;
        }

        //依次关闭客厅几个组的灯
        Json::ArrayIndex closeListSize = closeList.size();
        for(Json::ArrayIndex i = 0; i < closeListSize; ++i){
            qlibc::QData controlData = closeList.getArrayElement(i);
            SiteRecord::getInstance()->sendRequest2Site(bleSiteName, controlData, response);
            std::cout << "close: " << i << std::endl;
        }
    }

    std::cout << "-------send complete---------" << std::endl;

    while(true){
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
}

int main(int argc, char* argv[]){
    test();

    return 0;
}





















