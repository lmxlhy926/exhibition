
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <chrono>
#include <thread>
#include <cstdio>
#include "qlibc/Logging.h"
#include "qlibc/LogFile.h"
#include "socket/httplib.h"

using namespace qlibc;
using namespace httplib;

void allTypeTest(){

    LOG_INFO << "true: "<< true;
    LOG_INFO << "false: " << false;

    LOG_INFO << "short: " << static_cast<short>(100);
    LOG_INFO << "unsigned short: " << static_cast<unsigned short>(100);
    LOG_INFO << "int: " << static_cast<int>(100);
    LOG_INFO << "unsigned int: " << static_cast<unsigned int>(100);
    LOG_INFO << "long: " << static_cast<long>(100);
    LOG_INFO << "unsigned long: " << static_cast<unsigned long>(100);
    LOG_INFO << "long long: " << static_cast<long long>(100);
    LOG_INFO << "unsigned long long: " << static_cast<unsigned long long>(100);

    LOG_HLIGHT << "float: " << static_cast<float>(2.53);
    LOG_HLIGHT << "double: " << 2.53;

    int a[10] = {1, 2, 3};
    LOG_HLIGHT << "&a[0]: " << reinterpret_cast<const void*>(&a[0]);
    LOG_HLIGHT << "&a[1]: " << reinterpret_cast<const void*>(&a[1]);

    string str = "welcome to the world";
    LOG_HLIGHT << str;
}

void setOutputTest(){
    httplib::ThreadPool threadPool(10);
    LogFile lf(R"(D:\bywg\project\exhibition\unit\paramData\testSite\logout)", 1000* 30);

    qlibc::Logger::setOutput([&](const char* msg, size_t len, qlibc::Logger::LogLevel level){
        lf.append(msg, len);
    });


    threadPool.enqueue([](){
        for(int i = 0; i < 1000; i++){
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            LOG_INFO << 123456789;
        }
    });
    threadPool.enqueue([](){
        for(int i = 0; i < 1000; i++){
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            LOG_INFO << 123456789;
        }
    });
    threadPool.enqueue([](){
        for(int i = 0; i < 1000; i++){
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            LOG_HLIGHT << 123456789;
        }
    });

    threadPool.shutdown();
}

int main(int argc, char* argv[]){
    setOutputTest();

    std::cout << "-----------shutdown----------" << std::endl;

    while(true){
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }

    return 0;
}


















