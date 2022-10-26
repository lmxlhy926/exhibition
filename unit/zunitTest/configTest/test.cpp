
#include <iostream>
#include <functional>
#include <vector>
#include <thread>
#include <chrono>

#include "socket/socketServer.h"
#include "common/configParamUtil.h"



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
    configParamUtil* configPtr = configParamUtil::getInstance();
    configPtr->setConfigPath(string(argv[1]));
    QData data = configPtr->getBaseInfo();
    std::cout << data.toJsonString() << std::endl;

    data.setString("hello", "world");
    configParamUtil::getInstance()->saveBaseInfo(data);


    while(true)
        std::this_thread::sleep_for(std::chrono::seconds(10));

    return 0;
}




























