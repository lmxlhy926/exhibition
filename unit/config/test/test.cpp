
#include <iostream>
#include <functional>
#include <vector>
#include <thread>
#include <chrono>

#include "socket/socketServer.h"



using namespace std;
using namespace std::placeholders;

class ctest{
    int a = 0;
    int b = 0;
};

using FuncHandler = std::function<void(const Request& request, Response& response)>;

void push(const FuncHandler& handler){
    std::vector<FuncHandler> handlerVec;
    handlerVec.push_back(handler);
}

void test(socketServer& a, const Request& request, Response& response){
   std::cout << "----test----" << std::endl;
}


int main(int argc, char* argv[])
{
    socketServer c;
    Request request;
    Response response;
    int b;
    push([&](const Request& request, Response& response){
        test(c, request, response);
    });


//    bind(test, c, _1, _2)(request, response);


//    socketServer s;
//    Request request;
//    Response response;
//    bind(sceneListRequest_service_request_handler, s, _1, _2)(s, request, response);



    while(true)
        std::this_thread::sleep_for(std::chrono::seconds(10));

    return 0;
}




























