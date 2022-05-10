//
// Created by 78472 on 2022/5/9.
//

#ifndef EXHIBITION_SOCKETSERVER_H
#define EXHIBITION_SOCKETSERVER_H

#include <string>
#include "httplib.h"
#include "sockUtils.h"
#include "qlibc/QData.h"

using namespace std;

//存储和客户端建立连接的端点
class acceptNode{
private:
    string ip_; //对端的ip
    int port_;  //对端的port
    socket_t connectedSock = INVALID_SOCKET;  //用于和客户端通信的socket
    bool quit = false;
    std::shared_ptr<sockCommon::SocketStream> socketStream;
    std::shared_ptr<sockCommon::stream_line_reader> streamLineReader;

public:
    explicit acceptNode(const string& ip, int port, socket_t sock);

    ~acceptNode();

    bool isAlive() const;

    void close();

    ssize_t write(const char* buff, size_t size);

    bool write(const string& message);

    bool write(QData& message);

    bool readLine(string& str);
};

class objectPtrHolder {
private:
    std::map<const std::string, acceptNode *>  objectPtrMap;
    std::recursive_mutex mutex_;

public:
    objectPtrHolder()= default;

    //析构时释放所有被管理的对象的资源
    ~objectPtrHolder();

    //添加要管理的对象的指针
    void appendNew(const string& key, acceptNode* objPtr);

    /**
     * 返回key标识的对象的指针
     * @param key
     * @return 不存在则返回nullptr
     */
    acceptNode* findObject(const string& key);

    //是否存在key标识的对象
    bool existObject(const string& key);

    //将key标识的对象的指针移除管理map, 同时释放指针指向的对象的资源
    void eraseObject(const string& key);

    //对所有被管理的对象进行功能调用
    using objectFunction = std::function<void(const string& key, acceptNode*)>;
    void invokeOnAllObject(objectFunction func);
};

class socketServer {
private:
    string serverIp;
    int serverPort;
    socket_t serverSock_ = INVALID_SOCKET;
    bool bindAndListen = false;
    httplib::ThreadPool threadPool_;
    objectPtrHolder clients_;

public:
    explicit socketServer(int threadNum = 5);

    ~socketServer() = default;

    bool start(string& ip, int port, int socket_flags = AI_NUMERICHOST | AI_NUMERICSERV);

    void listen();

    bool postMessage(const string& message);

private:
    socket_t createServerSocket(string& ip, int port, int socket_flags);

    bool listen_internal();

    void process_socket(socket_t sock);
};


#endif //EXHIBITION_SOCKETSERVER_H
