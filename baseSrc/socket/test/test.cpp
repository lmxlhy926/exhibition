
#include <iostream>
#include <unordered_map>
#include <thread>
#include <chrono>
#include <string>
#include "socket/socketClient.h"



using namespace std;


void clientTest(){
    socketClient sc;
    sc.start("127.0.0.1", 60000, string("loginMessage"));
    sc.setUriHandler("hello", [](QData& message)->bool{
        std::cout << "--uriHandler-hello--:" << message.toJsonString(true) << std::endl;
    });

    while(true)
        std::this_thread::sleep_for(std::chrono::seconds(100));

}


void serverTest(){
    string ip = "127.0.0.1";
    int port = 60001;
//    socketServer server;
//    server.start(ip, 60001);
//    server.listen();
}


int main(int argc, char* argv[]){
    clientTest();

    std::cout << "-----main here-----" << std::endl;



    while(true)
        std::this_thread::sleep_for(std::chrono::seconds(100));

    return 0;
}






















