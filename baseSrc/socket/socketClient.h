//
// Created by 78472 on 2022/5/1.
//

#ifndef EXHIBITION_SOCKETCLIENT_H
#define EXHIBITION_SOCKETCLIENT_H

/**
 * 1. 创建后自动建立连接，成功后置标志connected
 * 2. 断开后自动建立连接
 * 3. 成功则，读取消息，放入线程池
 * 4. 成功则，可以主动发送消息
 */


#include "httplib.h"
#include <mutex>
#include <string>
#include <functional>
#include "sockUtils.h"
#include "messageHandler.h"


using namespace std;
class socketClient {
public:
    using afterConnectHandler = std::function<bool(const char* buff, int len)>;
private:
    socket_t sock_ = -1;
    std::string host_;
    int port_ = -1;
    string content;

    bool sockValid = false;
    bool quit = false;

    httplib::ThreadPool threadPool_;
    messageHandler receivedJsonHandler;
    std::shared_ptr<sockCommon::SocketStream> socketStream;
    std::shared_ptr<sockCommon::stream_line_reader> streamLineReader;
    afterConnectHandler func;

public:
    explicit socketClient(): threadPool_(10){}
    ~socketClient()= default;

    //启动client连接server
    bool start(const string& ip, int port, string loginMessage);

    bool stop();

    //是否和server依然处于连接状态
    bool isConnectionAlive();

    //向server发送消息
    bool sendMessage(const char* buff, int len);

    //向server发送消息
    bool sendMessage(const string& str);

    void setDefaultHandler(const JsonSocketHandler& defaultHandler);

    void setUriHandler(const string& uri, const JsonSocketHandler& jsHandler);

private:
    bool socketFdValid();

    bool establishConnection();

    void readLineAndHandle();

    void connectAndHandle();

    void shutdownAndCloseSocket();
};


#endif //EXHIBITION_SOCKETCLIENT_H
