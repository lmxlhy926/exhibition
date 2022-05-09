//
// Created by 78472 on 2022/5/9.
//

#ifndef EXHIBITION_SOCKETSERVER_H
#define EXHIBITION_SOCKETSERVER_H

#include <string>
#include "httplib.h"
#include "sockUtils.h"
#include "qlibc/QData.h"
#include "objecPtrtHolder.h"

using namespace std;

//存储和客户端建立连接的端点
class acceptNode{
private:
    string ip_; //对端的ip
    int port_;  //对端的port
    const socket_t connectedSock = INVALID_SOCKET;  //用于和客户端通信的socket
    bool quit = false;
    std::shared_ptr<sockCommon::SocketStream> socketStream;
    std::shared_ptr<sockCommon::stream_line_reader> streamLineReader;

public:
    explicit acceptNode(const string& ip, int port, socket_t sock);

    ~acceptNode() = default;

    bool isAlive() const;

    void close();

    ssize_t write(const char* buff, size_t size);

    bool write(const string& message);

    bool write(QData& message);

    bool readLine(string& str);
};



class socketServer {
private:
    string serverIp;
    int serverPort;
    socket_t serverSock_ = INVALID_SOCKET;
    httplib::ThreadPool threadPool_;
    objectPtrHolder<acceptNode> clients_;

public:
    explicit socketServer(int threadNum = 5);

    bool start(string& ip, int port, int socket_flags = AI_NUMERICHOST | AI_NUMERICSERV);

    void listen();

    bool postMessage(const string& message);

private:
    socket_t createServerSocket(string& ip, int port, int socket_flags);

    bool listen_internal();

    void process_socket(socket_t sock);
};


#endif //EXHIBITION_SOCKETSERVER_H
