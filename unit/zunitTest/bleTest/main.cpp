
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

void test1(){
    string bleSiteName = "ble_light";
    string LocalIp = "192.168.58.116";
    int bleSitePort = 9001;

    SiteRecord::getInstance()->addSite(bleSiteName, LocalIp, bleSitePort);
}


void groupControl(string group_id, string command_id, string command_para){
    qlibc::QData command, commandList, groupListItem, groupList, controlData;
    command.setString("command_id", command_id);
    command.setString("command_para", command_para);
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


//09C0
void control2(int delay){
    groupControl("0BC0", "power", "off");
    std::this_thread::sleep_for(std::chrono::seconds(2));

    groupControl("09C0", "power", "on");
    std::this_thread::sleep_for(std::chrono::seconds(delay));

    groupControl("09C0", "power", "off");
    groupControl("0AC0", "power", "on");
}


//05C0
void control4(int delay){
    groupControl("08C0", "power", "off");
    std::this_thread::sleep_for(std::chrono::seconds(2));

    groupControl("05C0", "power", "on");

    groupControl("05C0", "power", "off");
    groupControl("06C0", "power", "on");
    std::this_thread::sleep_for(std::chrono::seconds(delay));

    groupControl("06C0", "power", "off");
    groupControl("07C0", "power", "on");
}


//01C0
void control6(int delay){
    groupControl("04C0", "power", "off");
    std::this_thread::sleep_for(std::chrono::seconds(2));

    groupControl("01C0", "power", "on");

    groupControl("01C0", "power", "off");
    groupControl("02C0", "power", "on");
    std::this_thread::sleep_for(std::chrono::seconds(delay));

    groupControl("02C0", "power", "off");
    groupControl("03C0", "power", "on");
}



int main(int argc, char* argv[]){
    control4(5);
    return 0;
}





















