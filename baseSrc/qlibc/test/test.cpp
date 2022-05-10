

#include "qlibc/QData.h"
#include "qlibc/FileUtils.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <string>

using namespace std;
using namespace qlibc;

/**
 * 打印QData的类型，类中_value的地址，内容，大小，是否为空等信息
 * @param message ：提示信息
 * @param data
 */
void printValue(const string& message, QData& data){
    string type;
    switch(data.type()){
        case Json::nullValue:
            type = "Json::nullValue";
            break;
        case Json::stringValue:
            type = "Json::stringValue";
            break;
        case Json::objectValue:
            type = "Json::objectValue";
            break;
        case Json::arrayValue:
            type = "Json::arrayValue";
            break;
        case Json::intValue:
            type = "Json::intValue";
            break;
        case Json::uintValue:
            type = "Json::uintValue";
            break;
        case Json::realValue:
            type = "Json::realValue";
            break;
        case Json::booleanValue:
            type = "Json::booleanValue";
            break;
    }


    std::cout << message << ":>" << type << "--->" << &data.asValue()<< std::endl;
    std::cout << message << ":>" << data.toJsonString() << std::endl;
    std::cout << message << ":size>" << data.size() << std::endl;
    if(data.empty()){
        std::cout << message << ":>empty" << std::endl;
    }else{
        std::cout << message << ":>not empty" << std::endl;
    }
}

/**
 * 构造函数，拷贝构造函数，赋值操作符测试
 */
void test(){
    QData data;
    printValue("data", data);

    string str = R"({"hello":"world"})";
    QData data1(str);
    printValue("data1", data1);

    QData data2 = data1;
    printValue("data2", data2);

    QData data3(data2);
    printValue("data3", data3);

    Json::Value value(Json::arrayValue);
    value.append(1);
    QData data4(value);
    printValue("data4", data4);

    Json::Value value5 = 2;
    QData data5(value5);
    printValue("data", data5);
}

/**
 * 从文件读取json字符串测试
 */
void test1(){
    string path = R"(D:\bywg\project\exhibition\baseSrc\qlibc\test\test.json)";
    QData data;
    data.loadFromFile(path);
    printValue("data", data);

    Json::Value value;
    value["first"] = "first";
    data.setInitValue(value);
    printValue("data", data);
}

/**
 * removeMember, getMemberNames, clear测试
 */
void test2(){
    string path = R"(D:\bywg\project\exhibition\baseSrc\qlibc\test\test.json)";
    QData data;
    data.loadFromFile(path);
    printValue("data", data);

    data.removeMember("hello");
    printValue("data", data);
    auto members = data.getMemberNames();
    for(auto& v: members){
        std::cout << "members:>" << v << std::endl;
    }
    data.clear();
    printValue("data", data);

    Json::Value value = 2;
    QData data1(value);
    printValue("data1", data1);
    data1.clear();
}

void test3(){
    string path = R"(D:\bywg\project\exhibition\baseSrc\qlibc\test\test.json)";
    QData data;
    data.loadFromFile(path);
    std::cout << data.toJsonString(true) << std::endl;
    std::cout << "-----------------" << std::endl;
}

int main(int argc, char* argv[]){
    string dir = "/ab/";
    string fileName = "hello";

    string out = FileUtils::contactFileName(dir, fileName);
    std::cout << "out: " << out << std::endl;
    std::cout << "dir: " << dir << std::endl;

    while(true){
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }

    return 0;
}
















































