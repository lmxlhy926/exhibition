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


#include "common/httplib.h"
#include <mutex>
#include <string>
#include <functional>
#include <atomic>
#include "sockUtils.h"
#include "noncopyable.h"
#include "messageHandler.h"

using namespace std;

/**
 * socketclient
 *      1. start启动，掉线后自动重连
 *      2. stop关闭连接
 */

using afterConnectHandler = std::function<void()>;

class socketClient : noncopyable{
private:
    socket_t sock_ = INVALID_SOCKET;    //创建的客户端socket fd
    std::string ip_;        //服务器ip地址
    int port_ = -1;         //服务器端口号
    string loginMessage_;   //连接成功后上报的loginMessage
    std::atomic<bool> sockValid{false};     //端口号是否有效
    std::atomic<bool> quit{false};          //是否退出

    httplib::ThreadPool& threadPool_;      //线程池
    messageHandler receivedJsonHandler;    //处理消息的handler表
    std::unique_ptr<sockCommon::SocketStream> socketStream;
    std::unique_ptr<sockCommon::stream_line_reader> streamLineReader;
    std::recursive_mutex mutex_;

    afterConnectHandler func;

public:
    explicit socketClient(httplib::ThreadPool& threadPool) : threadPool_(threadPool){}

    void setAfterConnectHandler(afterConnectHandler handler);

    ~socketClient();

    //启动client连接server
    bool start(const string& ip, int port, string loginMessage);

    bool stop();

    //是否和server依然处于连接状态
    bool isConnectionAlive();

    //向server发送消息
    bool sendMessage(const char* buff, int len);

    //向server发送消息
    bool sendMessage(const string& str, bool withLF = false);

    void setDefaultHandler(const JsonSocketHandler& defaultHandler);

    void setUriHandler(const string& uri, const JsonSocketHandler& jsHandler);

private:
    /**
     * 和服务器建立连接，并发送登录信息
     * @return false: 和服务器建立连接失败; true:和服务器建立连接成功;
     */
    bool establishConnection();

    //阻塞读取一行内容并处理，如果返回则代表连接断开
    void readLineAndHandle();

    /**
     * 和服务器连接失败后，自动重连
     */
    void connectAndHandle();


    void shutdownAndCloseSocket();
};


#endif //EXHIBITION_SOCKETCLIENT_H
