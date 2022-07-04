//
// Created by 78472 on 2022/6/7.
//

#include "JsonCmd2Binary.h"

string JsonCmd2Binary::getBinaryString(QData &bleConfigData) {
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
    return binaryCommandString;
}

size_t JsonCmd2Binary::binaryString2binary(string &binaryString, unsigned char *buf, size_t size) {
    std::cout << "==>binaryString: " << binaryString << std::endl;
    BinaryBuf binaryBuf(buf, size);
    for(int i = 0; i < binaryString.size() / 2; i++){
        string charString = binaryString.substr(i * 2, 2);
        binaryBuf.append(charString);
    }
    return binaryBuf.size();
}


