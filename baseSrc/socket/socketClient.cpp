//
// Created by 78472 on 2022/5/1.
//

#include "socketClient.h"

socketClient::~socketClient(){
    shutdownAndCloseSocket();
}

bool socketClient::start(const string &ip, int port, string loginMessage) {
    ip_ = ip;
    port_ = port;
    loginMessage_ = std::move(loginMessage);
    quit.store(false);
    threadPool_.enqueue([&](){
        connectAndHandle();
    });
}

bool socketClient::stop(){
    quit.store(true);
    shutdownAndCloseSocket();
}

bool socketClient::isConnectionAlive() {
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    if(sockValid.load() && sockCommon::is_socket_alive(sock_))
        return true;
    return false;
}

bool socketClient::sendMessage(const char *buff, int len) {
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    if(isConnectionAlive()){
        return sockCommon::write_data(*socketStream, buff, len);
    }
    return false;
}

bool socketClient::sendMessage(const string &str) {
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    return sendMessage(str.c_str(), str.size());
}

void socketClient::setDefaultHandler(const JsonSocketHandler &defaultHandler) {
    receivedJsonHandler.setDefaultHandler(defaultHandler);
}

void socketClient::setUriHandler(const string &uri, const JsonSocketHandler &jsHandler) {
    receivedJsonHandler.setUriHandler(uri, jsHandler);
}

bool socketClient::establishConnection(){
    if(quit.load())    return false;
    sock_ = sockCommon::create_socket(ip_.c_str(), port_,
                                      [](socket_t sock2, struct addrinfo &ai)->bool{
                                          return sockCommon::connect(sock2, ai);
                                      });
    if(sock_ == INVALID_SOCKET){
        sockValid.store(false);
        std::cout << "failed in create socket client" << std::endl;
        return false;
    }
    sockValid.store(true);

    socketStream.reset(new sockCommon::SocketStream(sock_));
    streamLineReader.reset(new sockCommon::stream_line_reader(*socketStream));

    sendMessage(loginMessage_);
    return true;
}

void socketClient::readLineAndHandle() {
    while(true){
        if(quit.load() || !streamLineReader->getline()){   //读取失败即连接断开，结束读取操作
            std::cout << "finished to read....." << std::endl;
            shutdownAndCloseSocket();
            break;
        }

        std::cout << "received Line==>" << streamLineReader->ptr();
        QData data(streamLineReader->ptr(), streamLineReader->size() -1);
        if(data.type() != Json::nullValue){     //读取的字符串可解析为json对象
            string uri = data.getString("uri");
            receivedJsonHandler.disPatchMessage(uri, data);
        }

        //返回提示数据
        string str = "receive commit\n";
        sockCommon::write_data(*socketStream, str.c_str(), str.size());
    }
}

void socketClient::connectAndHandle(){
    while(true){
        if(quit.load())    break;
        if(establishConnection()){
            readLineAndHandle();
        }
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

void socketClient::shutdownAndCloseSocket(){
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    if(sock_ != INVALID_SOCKET){
        sockCommon::shutdown_socket(sock_);
        sockCommon::close_socket(sock_);
        std::cout << "sock:" << sock_ << " shutdownAndClosed---" << std::endl;
        sock_ = INVALID_SOCKET;
        sockValid.store(false);
    }
}
