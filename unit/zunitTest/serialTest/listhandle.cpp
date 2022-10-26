//
// Created by 78472 on 2022/9/9.
//

#include "qlibc/QData.h"
#include "formatTrans/statusEvent.h"
#include <list>


string deleteSpace(string str){
    for(auto pos = str.begin(); pos != str.end(); ){
        if(*pos == ' '){
            pos = str.erase(pos);
        }else{
            pos++;
        }
    }
    return str;
}

void printList(std::list<string>& sendList){
    std::cout << "sendList: " << std::endl;
    for(auto& elem : sendList){
        std::cout << "    " << elem << std::endl;
    }
}


void test(){
    std::list<string> sendList{
        deleteSpace("E8 FF 00 00 00 00 02 03 01 C0 82 5E FF FF 64 19 00 00 01"),
        deleteSpace("E8 FF 00 00 00 00 02 03 01 C0 82 5E FF FF 64 19 00 00 02"),
        deleteSpace("E8 FF 00 00 00 00 02 03 01 C0 82 5E FF FF 64 19 00 00 03"),
        deleteSpace("E8 FF 00 00 00 00 02 03 01 C0 82 5E FF FF 64 19 00 00 04"),
        deleteSpace("E8 FF 00 00 00 00 02 03 01 C0 82 5E FF FF 64 19 00 00 05"),
        deleteSpace("E8 FF 00 00 00 00 02 03 01 C0 82 5E FF FF 64 19 00 00 06"),

        deleteSpace("E8 FF 00 00 00 00 02 03 03 C0 82 5E FF FF 64 19 00 00 01"),
        deleteSpace("E8 FF 00 00 00 00 02 03 03 C0 82 5E FF FF 64 19 00 00 02"),
        deleteSpace("E8 FF 00 00 00 00 02 03 03 C0 82 5E FF FF 64 19 00 00 03"),

        deleteSpace("E8 FF 00 00 00 00 02 03 04 C0 82 5E FF FF 64 19 00 00 01"),
        deleteSpace("E8 FF 00 00 00 00 02 03 04 C0 82 5E FF FF 64 19 00 00 02"),
        deleteSpace("E8 FF 00 00 00 00 02 03 04 C0 82 5E FF FF 64 19 00 00 03"),

        deleteSpace("E8 FF 00 00 00 00 02 03 05 C0 82 5E FF FF 64 19 00 00 01"),
        deleteSpace("E8 FF 00 00 00 00 02 03 05 C0 82 5E FF FF 64 19 00 00 02"),
        deleteSpace("E8 FF 00 00 00 00 02 03 05 C0 82 5E FF FF 64 19 00 00 03"),

        deleteSpace("E8 FF 00 00 00 00 02 03 06 C0 82 5E FF FF 64 19 00 00 01"),
        deleteSpace("E8 FF 00 00 00 00 02 03 06 C0 82 5E FF FF 64 19 00 00 02"),
        deleteSpace("E8 FF 00 00 00 00 02 03 06 C0 82 5E FF FF 64 19 00 00 03"),

        deleteSpace("E8 FF 00 00 00 00 02 03 07 C0 82 5E FF FF 64 19 00 00 01"),
        deleteSpace("E8 FF 00 00 00 00 02 03 07 C0 82 5E FF FF 64 19 00 00 02"),
        deleteSpace("E8 FF 00 00 00 00 02 03 07 C0 82 5E FF FF 64 19 00 00 03"),

        deleteSpace("E8 FF 00 00 00 00 02 03 08 C0 82 5E FF FF 64 19 00 00 03"),

        deleteSpace("E8 FF 00 00 00 00 02 03 09 C0 82 5E FF FF 64 19 00 00 03")
    };

    for(auto& elem : sendList){
        std::cout << "elem: " << elem << std::endl;
    }


    //获取组列表, 循环遍历，删除后面的指令
//    qlibc::QData groupData;
//    groupData.loadFromFile(R"(D:\project\byjs\exhibition\unit\bleSite\serial\test\group.json)");
//    Json::Value::Members groupIdVec = groupData.getMemberNames();


    bleConfig* configPathPtr = bleConfig::getInstance();
    configPathPtr->setConfigPath(R"(D:\project\byjs\exhibition\unit\bleSite\)");
    qlibc::QData groupData = bleConfig::getInstance()->getGroupListData();
    Json::Value::Members groupIdVec = groupData.getMemberNames();

    //针对每个组名，只保留最近的一条指令
    for(auto& groupId : groupIdVec) {
        std::vector<string> sameCmdVec;
        for (auto & cmdString : sendList) {
            ReadBinaryString rs(cmdString);
            string groupIdExtract, opCodeExtract;
            rs.readBytes(8).read2Byte(groupIdExtract).read2Byte(opCodeExtract);
            if (groupIdExtract == groupId && opCodeExtract == "825E") {
                sameCmdVec.push_back(cmdString);
            }
        }
        if(sameCmdVec.size() > 1){
            for(int i = 0; i < sameCmdVec.size() -1; ++i){
                sendList.remove(sameCmdVec[i]);
            }
        }
    }
    printList(sendList);

    //书房、卧室的指令提前
    std::vector<string> specialGroup{"01C0", "03C0"};
    std::vector<string> priorityGroupCmd;
    for(auto & pos : sendList){
        ReadBinaryString rs(pos);
        string groupIdExtract, opCodeExtract;
        rs.readBytes(8).read2Byte(groupIdExtract).read2Byte(opCodeExtract);
        if(opCodeExtract == "825E"){
            for(auto& groupElem : specialGroup){
                if(groupIdExtract == groupElem){
                    priorityGroupCmd.push_back(pos);
                    break;
                }
            }
        }
    }
    for(auto & priorityElem : priorityGroupCmd){
        sendList.remove(priorityElem);
    }
    for(auto & priorityElem : priorityGroupCmd){
        sendList.push_back(priorityElem);
    }
    printList(sendList);

    if(sendList.size() > 0){
        std::cout << "last: " << sendList.back() << std::endl;
        sendList.pop_back();
        std::cout << "size: " << sendList.size() << std::endl;
    }
}




int main(int argc, char* argv[]){

    test();

    return 0;
}













