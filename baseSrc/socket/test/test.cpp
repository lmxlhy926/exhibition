
#include <iostream>
#include <unordered_map>
#include <map>
#include <thread>
#include <chrono>
#include "../objecPtrtHolder.h"


#include <string>
//#include "../socketServer.h"
//#include "../socketClient.h"


using namespace std;

#if 0
void test(){
    std::unordered_map<string, string> umap;
    umap.insert(std::make_pair("hello", "first"));
    umap.insert(std::make_pair("hello", "second"));

    std::cout << umap.find("hello")->second << std::endl;
}

void clientTest(){
    socketClient sc;
    sc.start("127.0.0.1", 60000, string("loginMessage"));
    sc.setUriHandler("hello", [](QData& message)->bool{
        std::cout << "--uriHandler-hello--:" << message.toJsonString(true) << std::endl;
    });

    while(true)
        std::this_thread::sleep_for(std::chrono::seconds(100));

}
#endif


int main(int argc, char* argv[]){
//    string ip = "127.0.0.1";
//    int port = 60001;
//    socketServer server;
//    server.start(ip, 60001);
//    server.listen();
//
//    std::cout << "-----main here-----" << std::endl;

    objectPtrHolder<int> obj;
    obj.existObject("sss");

    while(true)
        std::this_thread::sleep_for(std::chrono::seconds(100));

    return 0;
}






















