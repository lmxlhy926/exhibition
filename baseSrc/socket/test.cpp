

#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include "sockUtils.h"
#include "socketClient.h"
#include "qlibc/QData.h"


using namespace sockCommon;
using namespace std;

#if 0
int test1(){
    string ip = "127.0.0.1";
    int port = 60000;

    //创建客户端端点，并连接到服务器
    socket_t sock = sockCommon::create_socket(ip.c_str(), port,
                                              [](socket_t sock2, struct addrinfo &ai)->bool{
                                                  return sockCommon::connect(sock2, ai);
                                              });
    if(sock == INVALID_SOCKET){
        std::cout << "error in create socket client" << std::endl;
        return -1;
    }

    //创建socketstrean
    sockCommon::SocketStream sockStream(sock);

    //写内容，读内容
    string output = "helloworld";
    sockCommon::write_data(sockStream, output.c_str(), output.size());

    char buf[1024]{};
    int bufSize = 1024;
    sockCommon::stream_line_reader slr(sockStream, buf, bufSize);

    while(true){
        if(!slr.getline()){
            shutdown_socket(sock);
            close_socket(sock);
            std::cout << "---shutdown and closed first----" << std::endl;
            break;
        }

        if(slr.end_with_crlf()){
            std::cout << "readline==>" << slr.ptr();
        }else{
            std::cout  << "notaline===>" << slr.ptr() << std::endl;
        }

        sockCommon::write_data(sockStream, output.c_str(), output.size());
    }

    std::cout << "------connect lost-----" << std::endl;

    //判断连接是否断掉
    if(!sockCommon::is_socket_alive(sock)){
        std::cout << "-----sock is not alive------" << std::endl;
    }

    //主动关闭连接，重新进行连接

}

#endif





int main(int agrc, char* argv[]){
    socketClient sc;
    sc.start("127.0.0.1", 60000);
    sc.setUriHandler("hello", [](QData& message)->bool{
        std::cout << "--uriHandler-hello--:" << message.toJsonString(true) << std::endl;
    });

    while(true)
        std::this_thread::sleep_for(std::chrono::seconds(100));

    return 0;
}








































