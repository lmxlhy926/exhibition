
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <chrono>
#include <thread>
#include <cstdio>
#include "qlibc/Logging.h"

#include "socket/httplib.h"

using namespace qlibc;

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

    while(true){
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
}

void output(const char* msg, size_t len, Logger::LogLevel level){
    switch(level){
        case Logger::LogLevel::INFO:
            fprintf(stdout, WHITE "%s" COLOR_NONE, msg);
            fflush(stdout);
            break;
        case Logger::LogLevel::HLIGHT:
            fprintf(stdout, DEEP_GREEN "%s" COLOR_NONE, msg);
            fflush(stdout);
            break;
    }
}

int main(int argc, char* argv[]){
    httplib::ThreadPool threadPool(10);
    FILE* fp = fopen(R"(D:\bywg\project\exhibition\unit\paramData\testSite\logout.txt)", "a+");

    Logger::setOutput([&](const char* msg, size_t len, Logger::LogLevel level){
        switch(level){
            case Logger::LogLevel::INFO:
                fprintf(fp, WHITE "%s" COLOR_NONE, msg);
                fflush(fp);
                break;
            case Logger::LogLevel::HLIGHT:
                fprintf(fp, DEEP_GREEN "%s" COLOR_NONE, msg);
                fflush(fp);
                break;
        }
    });


    threadPool.enqueue([](){
        for(int i = 0; i < 1000; i++){
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            LOG_INFO << "***********************Q";
        }
    });
    threadPool.enqueue([](){
        for(int i = 0; i < 1000; i++){
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
            LOG_INFO << "***********************Q";
        }
    });
    threadPool.enqueue([](){
        for(int i = 0; i < 1000; i++){
            std::this_thread::sleep_for(std::chrono::milliseconds(3));
            LOG_INFO << "***********************Q";
        }
    });
    threadPool.enqueue([](){
        for(int i = 0; i < 1000; i++){
            std::this_thread::sleep_for(std::chrono::milliseconds(4));
            LOG_HLIGHT << "***********************Q";
        }
    });
    threadPool.enqueue([](){
        for(int i = 0; i < 1000; i++){
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            LOG_HLIGHT << "***********************Q";
        }
    });

    while(true){
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }

    return 0;
}


















