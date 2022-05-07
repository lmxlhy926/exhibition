//
// Created by 78472 on 2022/5/1.
//

#include "socketClient.h"

bool socketClient::start(const string &ip, int port, string loginMessage) {
    host_ = ip;
    port_ = port;
    content = loginMessage;
    quit = false;
    threadPool_.enqueue([&](){
        connectAndHandle();
    });
}

bool socketClient::stop(){
    shutdownAndCloseSocket();
    quit = true;
}

bool socketClient::isConnectionAlive() {
    if(socketFdValid() && sockCommon::is_socket_alive(sock_))
        return true;
    return false;
}

bool socketClient::sendMessage(const char *buff, int len) {
    if(isConnectionAlive()){
        return sockCommon::write_data(*socketStream, buff, len);
    }
    return false;
}

bool socketClient::sendMessage(const string &str) {
    return sendMessage(str.c_str(), str.size());
}

void socketClient::setDefaultHandler(const JsonSocketHandler &defaultHandler) {
    receivedJsonHandler.setDefaultHandler(defaultHandler);
}

void socketClient::setUriHandler(const string &uri, const JsonSocketHandler &jsHandler) {
    receivedJsonHandler.setUriHandler(uri, jsHandler);
}

bool socketClient::socketFdValid(){
    return sockValid;
}

bool socketClient::establishConnection(){
    if(quit)    return false;
    sock_ = sockCommon::create_socket(host_.c_str(), port_,
                                      [](socket_t sock2, struct addrinfo &ai)->bool{
                                          return sockCommon::connect(sock2, ai);
                                      });
    if(sock_ == INVALID_SOCKET){
        sockValid = false;
        std::cout << "failed in create socket client" << std::endl;
        return false;
    }
    sockValid = true;

    socketStream.reset(new sockCommon::SocketStream(sock_));
    streamLineReader.reset(new sockCommon::stream_line_reader(*socketStream));

    sendMessage(content);
    return true;
}

void socketClient::readLineAndHandle() {
    while(true){
        if(!streamLineReader->getline()){
            shutdownAndCloseSocket();
            break;
        }

        std::cout << "received Line==>" << streamLineReader->ptr();
        QData data(streamLineReader->ptr(), streamLineReader->size() -1);
        if(data.type() != Json::nullValue){
            string uri = data.getString("uri");
            receivedJsonHandler.disPatchMessage(uri, data);
        }

        string str = "messageBack\n";
        sockCommon::write_data(*socketStream, str.c_str(), str.size());
    }
}

void socketClient::connectAndHandle(){
    while(true){
        if(quit)    break;
        if(establishConnection()){
            readLineAndHandle();
        }
        std::this_thread::sleep_for(std::chrono::seconds(15));
    }
}

void socketClient::shutdownAndCloseSocket(){
    sockCommon::shutdown_socket(sock_);
    sockCommon::close_socket(sock_);
    sock_ = -1;
    sockValid = false;
    std::cout << "sock:" << sock_ << " shutdownAndClosed---" << std::endl;
}
