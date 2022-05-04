//
// Created by 78472 on 2022/5/1.
//

#include "socketClient.h"

bool socketClient::start(const string &ip, int port) {
    host_ = ip;
    port_ = port;
    connectAndReconnect();
}

void socketClient::readLine() {
    while(true){
        if(!slr->getline()){
            sockCommon::shutdown_socket(sock_);
            sockCommon::close_socket(sock_);
            std::cout << "---shutdown and closed first----" << std::endl;
            break;
        }

        if(slr->end_with_crlf()){
            std::cout << "readline==>" << slr->ptr();
            QData data(slr->ptr(), slr->size() -1);
            if(data.type() != Json::nullValue){
                string uri = data.getString("uri");
                jsonDataHandler.disPatchMessage(uri, data);
            }
        }else{
            std::cout  << "notaline===>" << slr->ptr() << std::endl;
        }

        string str = "helloworld\n";
        sockCommon::write_data(*sockst, str.c_str(), str.size());
    }

}


bool socketClient::connectAndReconnect(){
    sock_ = sockCommon::create_socket(host_.c_str(), port_,
                                      [](socket_t sock2, struct addrinfo &ai)->bool{
                                          return sockCommon::connect(sock2, ai);
                                      });
    if(sock_ == INVALID_SOCKET){
        std::cout << "error in create socket client" << std::endl;
        return false;
    }

    sockst.reset(new sockCommon::SocketStream(sock_));
    slr.reset(new sockCommon::stream_line_reader(*sockst, buffer, bufferSize));

    threadPool_.enqueue([&](){
        readLine();
        std::cout << "---lost and execute offlineHandler---" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
        connectAndReconnect();
    });

    return true;
}

bool socketClient::isActive() {
    return sockCommon::is_socket_alive(sock_);
}

bool socketClient::sendMessage(const char *buff, int len) {
    if(isActive()){
        return sockCommon::write_data(*sockst, buff, len);
    }
    return false;
}

bool socketClient::sendMessage(const string &str) {
    return sendMessage(str.c_str(), str.size());
}

void socketClient::setDefaultHandler(const JsonSocketHandler &defaultHandler) {
    jsonDataHandler.setDefaultHandler(defaultHandler);
}

void socketClient::setUriHandler(const string &uri, const JsonSocketHandler &jsHandler) {
    jsonDataHandler.setUriHandler(uri, jsHandler);
}
