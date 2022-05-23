//
// Created by 78472 on 2022/5/2.
//

#ifndef EXHIBITION_SOCKUTILS_H
#define EXHIBITION_SOCKUTILS_H

#include "httplib.h"

using namespace httplib;
namespace sockCommon{

    extern time_t connection_timeout_sec_;
    extern time_t connection_timeout_usec_;
    extern time_t read_timeout_sec_;
    extern time_t read_timeout_usec_;
    extern time_t write_timeout_sec_;
    extern time_t write_timeout_usec_;

    using BindOrConnect = std::function<bool(socket_t sock2, struct addrinfo &ai)>;

    /**
     * 创建一个socket作为客户端点或者服务器端点。
     * @tparam BindOrConnect
     * @param ip
     * @param port
     * @param bind_or_connect
     * @return 成功返回值>0; 失败返回INVALID_SOCKET;
     */
    socket_t create_socket(const char *ip, int port,
                           const BindOrConnect& bind_or_connect,
                           int socket_flags = AI_NUMERICHOST | AI_NUMERICSERV);

    //sock2端点连接addrinfo指定的服务器端口
    bool connect(socket_t sock2, struct addrinfo &ai);

    void shutdown_socket(socket_t sock);

    void close_socket(socket_t sock);

    /*
     * 试图读取该端点，从而判断该端点的连接是否正常
     * 正常的条件：
     *      1. 读取不到数据，但是通道连接是好的
     *      2. 数据通道关闭
     *      3. 有数据到达，则预读取一个测试通道连接
     */
    //todo 该方式是否只适用于客户端？
    bool is_socket_alive(socket_t sock);

    //监测sock端点是否有数据到达，超时返回
    ssize_t select_read(socket_t sock, time_t sec, time_t usec);

    //监测sock端点是否可以发送数据，超时返回
    ssize_t select_write(socket_t sock, time_t sec, time_t usec);

    //从sock端点读数据
    ssize_t read_socket(socket_t sock, void *ptr, size_t size, int flags);

    //向sock端点写数据
    ssize_t send_socket(socket_t sock, const void *ptr, size_t size, int flags);

    //设置sock端点属性
    void set_nonblocking(socket_t sock, bool nonblocking);

    bool is_connection_error();

    /**
     * 监测sock端点，在指定时间内是否可读或者可写
     * @param sock
     * @param sec
     * @param usec
     * @return 成功返回Error::Success
     */
    Error wait_until_socket_is_ready(socket_t sock, time_t sec, time_t usec);

    /**
     * 获取和sock端点连接的对端端点的ip,port.
     * @param sock
     * @param ip
     * @param port
     * @return 返回true,则ip和port的值有效; 返回false,则ip和port的值无效;
     */
    bool get_remote_ip_and_port(socket_t sock, std::string &ip, int &port);

    /**
     * 返回addr所表示的端点的ip,port。
     * @param addr
     * @param addr_len
     * @param ip
     * @param port
     * @return 返回true，则ip,port的值有效; 返回false,则ip和port代表的值无效;
     */
    bool get_ip_and_port(const struct sockaddr_storage &addr,
                                socklen_t addr_len, std::string &ip,
                                int &port);


    /*
     * 从sock端点中读取数据，向sock端点写入数据
     */
    class SocketStream : public Stream {
    public:
        explicit SocketStream(socket_t sock, time_t read_timeout_sec = CPPHTTPLIB_READ_TIMEOUT_SECOND,
                     time_t read_timeout_usec = CPPHTTPLIB_READ_TIMEOUT_USECOND,
                     time_t write_timeout_sec = CPPHTTPLIB_WRITE_TIMEOUT_SECOND,
                     time_t write_timeout_usec = CPPHTTPLIB_WRITE_TIMEOUT_USECOND);
        ~SocketStream() override;

        /*
         * 1. 读缓存中有数据
         * 2. 阻塞等待，有数据可以读取
         * 如果返回false, 则意味着和对端的连接通道被关闭。
         */
        bool is_readable() const override;

        //是否可写,立即返回
        bool is_writable() const override;

        /**
         * 读缓存中有数据则读取缓存中的数据，读缓存中无数据则阻塞读取
         * @param ptr
         * @param size
         * @return 负值则代表连接通道关闭; 正值代表实际读取到的值。
         */
        ssize_t read(char *ptr, size_t size) override;

        ssize_t write(const char *ptr, size_t size) override;

        void get_remote_ip_and_port(std::string &ip, int &port) const override;

        socket_t socket() const override;

    private:
        socket_t sock_;
        time_t read_timeout_sec_;
        time_t read_timeout_usec_;
        time_t write_timeout_sec_;
        time_t write_timeout_usec_;

        std::vector<char> read_buff_;        //读取数据缓存
        size_t read_buff_off_ = 0;           //当前位置
        size_t read_buff_content_size_ = 0;  //读取的内容大小

        static const size_t read_buff_size_ = 1024 * 4;
    };

    bool write_data(Stream &strm, const char *d, size_t l);


    /*
     * 阻塞读取一行内容，返回false则代表连接通道关闭
     */
    class stream_line_reader {
    public:
        explicit stream_line_reader(Stream &strm) : strm_(strm){};
        const char *ptr() const;        //返回读取内容
        size_t size() const;            //读取的内容字节数
        bool end_with_crlf() const;     //是否以特定字符结尾
        bool getline();                 //阻塞读取一行内容，返回false则代表通道关闭

    private:
        void append(char c);

        Stream &strm_;
        char fixed_buffer_[1024]{};
        const size_t fixed_buffer_size_ = 1024;
        size_t fixed_buffer_used_size_ = 0;
        std::string glowable_buffer_;
    };

}

#endif //EXHIBITION_SOCKUTILS_H
