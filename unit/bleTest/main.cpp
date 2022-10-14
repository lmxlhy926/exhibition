

#include <iostream>
#include "common/httpUtil.h"

int main(int argc, char* argv[]){
    string bleSiteName = "ble_light";
    string LocalIp = "192.168.58.118";
    int bleSitePort = 9001;

    SiteRecord::getInstance()->addSite(bleSiteName, LocalIp, bleSitePort);

    qlibc::QData group1, group2, numdata, response;
    group1.loadFromFile(R"(D:\bywg\project\exhibition\unit\bleTest\group1.json)");
    group2.loadFromFile(R"(D:\bywg\project\exhibition\unit\bleTest\group2.json)");
    numdata.loadFromFile(R"(D:\bywg\project\exhibition\unit\bleTest\num.json)");


    qlibc::QData group1Open = group1.getData("open");
    qlibc::QData group1Close = group1.getData("close");
    qlibc::QData group2Open = group2.getData("open");
    qlibc::QData group2Close = group2.getData("close");

    int num = numdata.getInt("num");

    for(int i = 0; i < num; ++i){
        std::cout << "i: " << i << std::endl;
        SiteRecord::getInstance()->sendRequest2Site(bleSiteName, group1Open, response);
        SiteRecord::getInstance()->sendRequest2Site(bleSiteName, group2Open, response);
        SiteRecord::getInstance()->sendRequest2Site(bleSiteName, group1Close, response);
        SiteRecord::getInstance()->sendRequest2Site(bleSiteName, group2Close, response);
    }

    while(true){
        std::this_thread::sleep_for(chrono::seconds(10));
    }

    return 0;
}





















