
#include <iostream>
#include "common/httpUtil.h"
#include "qlibc/QData.h"
#include <regex>


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

void groupControl(string group_id, string command_id, int command_para){
    qlibc::QData command, commandList, groupListItem, groupList, controlData;
    command.setString("command_id", command_id);
    command.setInt("command_para", command_para);
    command.setInt("transTime", 0);
    commandList.append(command);
    groupListItem.setString("group_id", group_id);
    groupListItem.putData("command_list", commandList);
    groupList.append(groupListItem);
    controlData.setString("service_id", "control_group");
    controlData.putData("request", qlibc::QData().putData("group_list", groupList));

    string bleSiteName = "ble_light";
    string LocalIp = "127.0.0.1";
    int bleSitePort = 9001;
    qlibc::QData controlRes;
    httpUtil::sitePostRequest(LocalIp, bleSitePort, controlData, controlRes);
}



void control4(int delay){
    groupControl("08C0", "luminance", 100);
    std::this_thread::sleep_for(std::chrono::seconds(2));

    groupControl("05C0", "luminance", 255);
    std::this_thread::sleep_for(std::chrono::seconds(delay));

    groupControl("06C0", "luminance", 255);
    groupControl("05C0", "luminance", 100);
    std::this_thread::sleep_for(std::chrono::seconds(delay));

    groupControl("07C0", "luminance", 255);
    groupControl("06C0", "luminance", 100);
}


void blink(){
    while(true){
        groupControl("FFFF", "luminance", 50);
        sleep(3);
        groupControl("FFFF", "luminance", 255);
    }
}



int main(int argc, char* argv[]){
    blink();
    return 0;
}





















