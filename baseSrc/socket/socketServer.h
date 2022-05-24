//
// Created by 78472 on 2022/5/9.
//

#ifndef EXHIBITION_SOCKETSERVER_H
#define EXHIBITION_SOCKETSERVER_H

#include <string>
#include "httplib.h"
#include "sockUtils.h"
#include "qlibc/QData.h"
#include "noncopyable.h"

using namespace std;
using namespace qlibc;
using namespace httplib;

/**
 * socketServer服务器：
 *      1. 保存多个连接端点的信息
 *      2. 接收信息，调用相应的回调进行处理
 *      3. 发送消息（指定端点或者广播发送）
 */


//存储和客户端建立连接的端点
/**
 * 维护一个连接端点，通过Node和对端进行通信，断开连接时自动销毁端点
 */
class acceptNode{
private:
    string ip_; //通信对端的ip
    int port_;  //通信对端的port
    socket_t connectedSock = INVALID_SOCKET;  //用于和对端通信的socket
    std::unique_ptr<std::recursive_mutex> mutex_;
    std::unique_ptr<sockCommon::SocketStream> socketStream;
    std::unique_ptr<sockCommon::stream_line_reader> streamLineReader;

public:
    explicit acceptNode(const string& ip, int port, socket_t sock);

    ~acceptNode();

    //和对端的连接是否处于有效状态
    bool isAlive();

    //关闭connectedSock，结束和对端的连接
    void close();

    ssize_t write(const char* buff, size_t size);

    bool write(const string& message);

    bool write(QData& message);

    bool readLine(string& str);
};


/**
 * 存储acceptNode，可进行：插入，删除，查找等操作
 */
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

    //清空所有连接端点
    void clear();

    //对所有被管理的对象进行功能调用
    using objectFunction = std::function<void(const string& key, acceptNode*)>;
    void invokeOnAllObject(objectFunction func);
};


/**
 * socketServer服务器：供客户端连接，与客户端进行通信
 */
class socketServer: noncopyable{
private:
    string serverIp;    //服务器ip
    int serverPort;     //服务器端口号
    socket_t serverSock_ = INVALID_SOCKET;  //服务器监听端点
    bool isRunning = false;
    httplib::ThreadPool threadPool_;
    objectPtrHolder clients_;  //连接到服务器的客户端
    std::recursive_mutex mutex_;

public:
    explicit socketServer(size_t n = 10);

    ~socketServer() = default;

    /**
     * 指定ip,端口号，解析ip和端口号的方式; 绑定/监听该服务端口
     * @param ip
     * @param port
     * @param socket_flags
     * @return 监听成功/失败
     */
    bool start(string& ip, int port, int socket_flags = AI_NUMERICHOST | AI_NUMERICSERV);

    void stop();

    bool postMessage(const string& message);

private:
    //创建serverSocket，并且bind,listen
    socket_t createServerSocket(string& ip, int port, int socket_flags);

    //监听是否有端点连接serverSocket
    bool listen_internal();

    void process_socket(socket_t sock);
};


#endif //EXHIBITION_SOCKETSERVER_H
