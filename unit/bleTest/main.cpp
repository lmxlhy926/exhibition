

#include <iostream>
#include "common/httpUtil.h"

int main(int argc, char* argv[]){
    string bleSiteName = "ble_light";
    string LocalIp = "192.168.58.118";
    int bleSitePort = 9001;

    SiteRecord::getInstance()->addSite(bleSiteName, LocalIp, bleSitePort);

    qlibc::QData open, close, response;
    open.loadFromFile(R"(D:\bywg\project\exhibition\unit\bleTest\open.json)");
    close.loadFromFile(R"(D:\bywg\project\exhibition\unit\bleTest\close.json)");

    for(int i = 0; i < 10; ++i){
        SiteRecord::getInstance()->sendRequest2Site(bleSiteName, open, response);
        SiteRecord::getInstance()->sendRequest2Site(bleSiteName, close, response);
    }

    return 0;
}





















