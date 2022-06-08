//
// Created by 78472 on 2022/6/7.
//

#include "bleControl.h"
#include "../bleConfigParam.h"
#include <iostream>


void set_light_turnOnOff(qlibc::QData& lightData, string& command, string& destAddress){
    lightData.asValue()["commonBase"]["param"]["ADDRESS_DEST"] = destAddress;
    lightData.asValue()["commonBase"]["param"]["OPERATION"] = command;
}


void control(qlibc::QData& data){
    std::cout << "===>data: " << data.toJsonString() << std::endl;
    string pseudoCommand  = data.getData("request").getString("command");
    string address = data.getData("request").getString("device_id");
    qlibc::QData thisBleConfigData = bleConfigParam::getInstance()->getBleParamData();;
    set_light_turnOnOff(thisBleConfigData, pseudoCommand, address);

    std::vector<string> commonBaseParamOrderVec;
    qlibc::QData commonBaseParamOrder = thisBleConfigData.getData("commonBase").getData("paramOrder");
    for(int i = 0; i < commonBaseParamOrder.size(); i++){
        commonBaseParamOrderVec.push_back(commonBaseParamOrder.getArrayElement(i).asValue().asString());
    }

    string binaryCommandString;

    for(auto &elem : commonBaseParamOrderVec){
        if(elem != "OPERATION"){
            binaryCommandString += thisBleConfigData.getData("commonBase").getData("param").getString(elem);
        }else if(elem == "OPERATION"){
            string controlCommand = thisBleConfigData.getData("commonBase").getData("param").getString(elem);
            Json::Value::Members operationKeyMembers = thisBleConfigData.getData("OPERATION").getMemberNames();

            for(auto& key : operationKeyMembers){
                qlibc::QData cmd = thisBleConfigData.getData("OPERATION").getData(key).getData("cmd");

                for(int i = 0; i < cmd.size(); i++){
                    if(controlCommand == cmd.getArrayElement(i).asValue().asString()){
                        //匹配到命令
                        qlibc::QData controlCmdData = thisBleConfigData.getData("OPERATION").getData(key).getData("cmdParam").getData(controlCommand);
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
}

