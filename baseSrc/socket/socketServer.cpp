//
// Created by 78472 on 2022/5/9.
//

#include "socketServer.h"

#include <memory>

acceptNode::acceptNode(const string &ip, int port, const socket_t sock) :
                                   ip_(ip), port_(port), connectedSock(sock){
    if(!isAlive())  return;
    mutex_ = std::make_unique<std::recursive_mutex>();
    socketStream = std::make_unique<sockCommon::SocketStream>(sock);
    streamLineReader = std::make_unique<sockCommon::stream_line_reader>(*socketStream);
}

acceptNode::~acceptNode(){
    close();
}

bool acceptNode::isAlive() const{
    std::lock_guard<std::recursive_mutex> lg(*mutex_);
    return connectedSock != INVALID_SOCKET;
}

void acceptNode::close(){
    std::lock_guard<std::recursive_mutex> lg(*mutex_);
    if(isAlive()){
        sockCommon::shutdown_socket(connectedSock);
        sockCommon::close_socket(connectedSock);
        connectedSock = INVALID_SOCKET;
    }
}

ssize_t acceptNode::write(const char *buff, size_t size) {
    std::lock_guard<std::recursive_mutex> lg(*mutex_);
    if(!isAlive())  return -1;
    return socketStream->write(buff, size);
}

bool acceptNode::write(const string &message) {
    std::lock_guard<std::recursive_mutex> lg(*mutex_);
    if(!isAlive())  return false;
    size_t length = message.length();
    return  socketStream->write(message.c_str(), message.length()) == length;
}

bool acceptNode::write(QData &message) {
    std::lock_guard<std::recursive_mutex> lg(*mutex_);
    string buff = message.toJsonString();
    buff += "\n";
    return write(buff);
}

bool acceptNode::readLine(string& str){
    std::lock_guard<std::recursive_mutex> lg(*mutex_);
    if(!isAlive())  return false;

    if(streamLineReader->getline()){
        str.assign(streamLineReader->ptr(), streamLineReader->size() -1);
        return true;
    }
    close();
    return false;
}


objectPtrHolder::~objectPtrHolder() {
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    for(auto& elem : objectPtrMap){
        delete elem.second;
    }
}


void objectPtrHolder::appendNew(const string &key, acceptNode *objPtr) {
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    if(objPtr == nullptr || key.empty())
        return;
    objectPtrMap.insert(std::make_pair(key, objPtr));
}

acceptNode *objectPtrHolder::findObject(const string &key) {
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    if(key.empty())
        return nullptr;
    auto ret = objectPtrMap.find(key);
    if(ret != objectPtrMap.end())
        return ret->second;
    else
        return nullptr;
}

bool objectPtrHolder::existObject(const string &key) {
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    if(key.empty())
        return false;
    auto ret = objectPtrMap.find(key);
    if(ret == objectPtrMap.end())
        return false;
    else
        return true;
}

void objectPtrHolder::eraseObject(const string &key) {
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    if(key.empty())
        return;
    auto ret = objectPtrMap.find(key);
    if(ret == objectPtrMap.end())
        return;
    else{
        delete ret->second;
        objectPtrMap.erase(key);
        return;
    }
}

void objectPtrHolder::invokeOnAllObject(objectPtrHolder::objectFunction func) {
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    for(auto & elem : objectPtrMap){
        func(elem.first, elem.second);
    }
}

socketServer::socketServer(httplib::ThreadPool& threadPool) : threadPool_(threadPool) {}

bool socketServer::start(string& ip, int port, int socket_flags) {
    serverIp = ip;
    serverPort = port;
    serverSock_ = createServerSocket(serverIp, serverPort, socket_flags);
    if(serverSock_ != INVALID_SOCKET){
        bindAndListen = true;
        return true;
    }
    return false;
}

void socketServer::listen(){
    threadPool_.enqueue([&](){
        if(bindAndListen)
            listen_internal();
    });
}

bool socketServer::postMessage(const string& message) {
    clients_.invokeOnAllObject([&](const string& key, acceptNode* node){
        node->write(message);
    });
    return true;
}

socket_t socketServer::createServerSocket(string& ip, int port, int socket_flags) {
    return sockCommon::create_socket(ip.c_str(), port,
                            [&](socket_t sock, struct addrinfo &ai)->bool{
                                                if(::bind(sock, ai.ai_addr, ai.ai_addrlen) != 0)
                                                    return false;
                                                else
                                                    return ::listen(sock, 10) == 0;
                                            }, socket_flags);
}


bool socketServer::listen_internal() {
    std::cout << "---server<ip: " << serverIp << ", port: " << serverPort << "> start to listen....." << std::endl;
    while(true){
        auto ret = sockCommon::select_read(serverSock_, 0, 100000);
        if(ret == 0){   //timeout
            continue;
        }

        socket_t acceptSock = accept(serverSock_, nullptr, nullptr);
        if(acceptSock == INVALID_SOCKET){
            if(errno == EMFILE){
                // The per-process limit of open file descriptors has been reached.
                // Try to accept new connections after a short sleep.
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
            }
            if(serverSock_ != INVALID_SOCKET){
                sockCommon::shutdown_socket(serverSock_);
                sockCommon::close_socket(serverSock_);
                serverSock_ = INVALID_SOCKET;
                bindAndListen = false;
                return false;
            }else{
                ;
            }
            break;
        }

        //开启线程处理客户端连接
        threadPool_.enqueue([&](){
            process_socket(acceptSock);
        });
    }
    return false;
}

//处理和客户端建立的连接
void socketServer::process_socket(socket_t sock) {
    string ip;
    int port;
    if(sockCommon::get_remote_ip_and_port(sock, ip, port)){
        string key = ip + std::to_string(port) + std::to_string(sock);
        acceptNode* node = new acceptNode(ip, port, sock);
        clients_.appendNew(key, node);
        std::cout << "---client<ip: " << ip << ", port: " << port << "> connetc to the server......" << std::endl;

        while(true){
            string aReadedLine;
            bool readRet = node->readLine(aReadedLine);     //阻塞读取
            if(readRet){    //读取成功
                std::cout << "received: " << aReadedLine << std::endl;
                continue;
            }else{  //和客户端断开连接
                clients_.eraseObject(key);
                std::cout << "---client<ip: " << ip << ", port: " << port << "> disconnetced......" << std::endl;
                break;
            }
        }
    }

    //关闭和客户端建立的连接
    sockCommon::shutdown_socket(sock);
    sockCommon::close_socket(sock);
}


