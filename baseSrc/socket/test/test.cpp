
#include <iostream>
#include <unordered_map>
#include <thread>
#include <chrono>
#include <string>
#include "socket/socketClient.h"
#include "socket/socketServer.h"



using namespace std;


void clientTest(){
    httplib::ThreadPool threadPool(100);
    socketClient sc(threadPool);
    sc.start("127.0.0.1", 60000, "loginMessage");
    sc.setUriHandler("hello", [](QData& message)->bool{
        std::cout << "--uriHandler-hello--:" << message.toJsonString(true) << std::endl;
    });
    threadPool.enqueue([&](){
        while(true){
            std::this_thread::sleep_for(std::chrono::seconds(5));
            sc.sendMessage("hello");
        }
    });

    std::this_thread::sleep_for(std::chrono::seconds(30));
    sc.stop();
    std::cout << "client stopped....." << std::endl;
//    std::this_thread::sleep_for(std::chrono::seconds(10));
//    sc.start("127.0.0.1", 60000, "loginMessage");

    while(true)
        std::this_thread::sleep_for(std::chrono::seconds(100));

}


void serverTest(){
    string ip = "127.0.0.1";
    int port = 60003;
    socketServer server;
    if(server.start(ip, 60001)){
        std::cout << "---bind successfully---" << std::endl;
        server.listen();
    }
    while(true){
        std::this_thread::sleep_for(std::chrono::seconds(5));
        server.postMessage("hello");
    }

}


int main(int argc, char* argv[]){
    clientTest();



    return 0;
}






















