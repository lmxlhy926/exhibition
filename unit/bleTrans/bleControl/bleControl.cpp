//
// Created by 78472 on 2022/6/7.
//

#include "bleControl.h"
#include <cstdlib>
#include <exception>
#include "../bleConfigParam.h"
#include <iostream>

struct BinaryBuf{
    unsigned binaryBuf[100];
    size_t size = 0;

    void append(string& charString){
        int charInt = 0;
        try{
            charInt =  std::stoi(charString, nullptr, 16);
        }catch(std::exception& e){
            charInt = 0;
        }
        binaryBuf[size] = static_cast<unsigned char>(charInt);
        size++;
    }
};

void binaryString2binary(string& binaryString, struct BinaryBuf& binaryBuf){
    for(int i = 0; i < binaryString.size() / 2; i++){
        string charString = binaryString.substr(i * 2, 2);
        std::cout << "charString: " << charString << std::endl;
        binaryBuf.append(charString);
    }
}

string getBinaryString(qlibc::QData& bleConfigData){
    std::vector<string> commonBaseParamOrderVec;
    qlibc::QData commonBaseParamOrder = bleConfigData.getData("commonBase").getData("paramOrder");
    for(int i = 0; i < commonBaseParamOrder.size(); i++){
        commonBaseParamOrderVec.push_back(commonBaseParamOrder.getArrayElement(i).asValue().asString());
    }

    string binaryCommandString;

    for(auto &elem : commonBaseParamOrderVec){
        if(elem != "OPERATION"){
            binaryCommandString += bleConfigData.getData("commonBase").getData("param").getString(elem);
        }else if(elem == "OPERATION"){
            string controlCommand = bleConfigData.getData("commonBase").getData("param").getString(elem);
            Json::Value::Members operationKeyMembers = bleConfigData.getData("OPERATION").getMemberNames();

            for(auto& key : operationKeyMembers){
                qlibc::QData cmd = bleConfigData.getData("OPERATION").getData(key).getData("cmd");

                for(int i = 0; i < cmd.size(); i++){
                    if(controlCommand == cmd.getArrayElement(i).asValue().asString()){
                        //匹配到命令
                        qlibc::QData controlCmdData = bleConfigData.getData("OPERATION").getData(key).getData("cmdParam").getData(controlCommand);
                        binaryCommandString += controlCmdData.getString("opCode");

                        qlibc::QData param = controlCmdData.getData("param");
                        qlibc::QData paramOrder = controlCmdData.getData("paramOrder");
                        for(int j = 0; j < paramOrder.size(); j++){
                            string paramKey = paramOrder.getArrayElement(j).asValue().asString();
                            if(!param.getString(paramKey).empty())
                                binaryCommandString += param.getString(paramKey);
                        }
                        break;
                    }
                }
            }
        }
    }

    std::cout << "==>binaryCommandString: " << binaryCommandString << std::endl;

    return binaryCommandString;
}


void set_light_turnOnOff(qlibc::QData& lightData, string& command, string& destAddress){
    lightData.asValue()["commonBase"]["param"]["ADDRESS_DEST"] = destAddress;
    lightData.asValue()["commonBase"]["param"]["OPERATION"] = command;
}


void bleControl(qlibc::QData& data){
    string pseudoCommand  = data.getData("request").getString("command");

    string binaryControlString;
    if(pseudoCommand == "scan"){
        binaryControlString = "E9FF00";

    }else if(pseudoCommand == "addDevice"){
        binaryControlString = "E9FF08";
        binaryControlString += data.getData("request").getString("device_id");

    }else if(pseudoCommand == "deleteDevice"){

    }
    std::cout << "==>binaryControlString: " << binaryControlString << std::endl;

    struct BinaryBuf binaryBuf{};
    binaryString2binary(binaryControlString, binaryBuf);

    for(int i = 0; i < binaryBuf.size; i++){
        printf("==>%2x\n", binaryBuf.binaryBuf[i]);
    }
}


void bleCommand(qlibc::QData& data){
    std::cout << "===>data: " << data.toJsonString() << std::endl;
    string pseudoCommand  = data.getData("request").getString("command");
    string address = data.getData("request").getString("device_id");
    qlibc::QData thisBleConfigData = bleConfigParam::getInstance()->getBleParamData();

    if(pseudoCommand == "turnOn" || pseudoCommand == "turnOff"){
        set_light_turnOnOff(thisBleConfigData, pseudoCommand, address);
    }

    string binaryString = getBinaryString(thisBleConfigData);

    struct BinaryBuf binaryBuf{};
    binaryString2binary(binaryString, binaryBuf);

    for(int i = 0; i < binaryBuf.size; i++){
        printf("==>%2x\n", binaryBuf.binaryBuf[i]);
    }
}



