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
        if(!streamLineReader->getline()){
            sockCommon::shutdown_socket(sock_);
            sockCommon::close_socket(sock_);
            std::cout << "---shutdown and closed first----" << std::endl;
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


bool socketClient::connectAndReconnect(){
    sock_ = sockCommon::create_socket(host_.c_str(), port_,
                                      [](socket_t sock2, struct addrinfo &ai)->bool{
                                          return sockCommon::connect(sock2, ai);
                                      });
    if(sock_ == INVALID_SOCKET){
        std::cout << "error in create socket client" << std::endl;
        return false;
    }

    socketStream.reset(new sockCommon::SocketStream(sock_));
    streamLineReader.reset(new sockCommon::stream_line_reader(*socketStream));

    threadPool_.enqueue([&](){
        readLine();
//        std::cout << "---lost and execute offlineHandler---" << std::endl;
//        std::this_thread::sleep_for(std::chrono::seconds(2));
//        connectAndReconnect();
    });

    return true;
}

bool socketClient::isConnectionAlive() {
    return sockCommon::is_socket_alive(sock_);
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
