
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
    sc.setUriHandler("hello", [](QData& message)->bool{
        std::cout << "--uriHandler-hello--:" << message.toJsonString(true) << std::endl;
    });
    sc.start("127.0.0.1", 60003, "loginMessage");

//    threadPool.enqueue([&](){
//        while(true){
//            std::this_thread::sleep_for(std::chrono::seconds(5));
//            sc.sendMessage("hello");
//        }
//    });

//    std::this_thread::sleep_for(std::chrono::seconds(30));
//    sc.stop();
//    std::cout << "client stopped....." << std::endl;
//    std::this_thread::sleep_for(std::chrono::seconds(10));
//    sc.start("127.0.0.1", 60000, "loginMessage");

    while(true)
        std::this_thread::sleep_for(std::chrono::seconds(100));

}

void clientTest1(){
    string ip_ = "127.0.0.1";
    int port_ = 60003;
    socket_t sock_;

    while(true){
         sock_ = sockCommon::create_socket(ip_.c_str(), port_,
                                                   [](socket_t sock2, struct addrinfo &ai)->bool{
                                                       return sockCommon::connect(sock2, ai);
                                                   });
        if(sock_ == INVALID_SOCKET){
            std::cout << "failed in create socket client" << std::endl;
        }else{
            std::cout << "success in create socket client" << std::endl;
            break;
        }
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }


    sockCommon::SocketStream socketStream(sock_);
    sockCommon::stream_line_reader streamLineReader(socketStream);

    while(true){
        if(!streamLineReader.getline()){
            std::cout << "connet failed" << std::endl;
            break;
        }else{
            std::cout << "-->: " << streamLineReader.ptr() << std::endl;
        }
    }


    while(true){
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }

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
    clientTest1();

    return 0;
}






















