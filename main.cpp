#include <iostream>
#include "qlibc/StringUtils.h"


void printType(Json::ValueType type){
    switch(type){
        case Json::intValue:
            std::cout << "intValue" << endl;
            break;
        case Json::uintValue:
            std::cout << "uintValue" << endl;
            break;
        case Json::realValue:
            std::cout << "realValue" << endl;
            break;
        case Json::stringValue:
            std::cout << "stringValue" << endl;
            break;
        case Json::booleanValue:
            std::cout << "booleanValue" << endl;
            break;
        case Json::objectValue:
            std::cout << "objectValue" << endl;
            break;
        case Json::arrayValue:
            std::cout << "arrayValue" << endl;
            break;
        case Json::nullValue:
            std::cout << "nullValue" << endl;
            break;
    }
}

void parseFromFileTest(){
    string fileName = R"(D:\bywg\project\exhibition\test\c.json)";
    Json::Value value;
    if(StringUtils::parseFromFile(fileName, value)){
        printType(value.type());
        std::cout << "parse success" << endl;
    }else{
        std::cout << "parse failed" << endl;
    }
}

void valueToJsonstringTest(){
    Json::Value value(Json::objectValue);
    string ret;
    StringUtils::valueToJsonString(value, ret);
    std::cout << "ret:" << ret << endl;
}

void parseJsonTest(){
    string str = "slsl";
    Json::Value value;
    if(StringUtils::parseJson(str, value)){
        std::cout << "parse success" << std::endl;
        printType(value.type());
    }

    string out;
    StringUtils::valueToJsonString(value, out);
    std::cout << out << std::endl;
}


using namespace std;
int main() {
    Json::Value value(Json::objectValue);
    std::cout << value.toStyledString();






    return 0;
}
