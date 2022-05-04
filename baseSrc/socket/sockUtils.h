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

    bool connect(socket_t sock2, struct addrinfo &ai);

    void shutdown_socket(socket_t sock);

    void close_socket(socket_t sock);

    bool is_socket_alive(socket_t sock);

    ssize_t select_read(socket_t sock, time_t sec, time_t usec);

    ssize_t select_write(socket_t sock, time_t sec, time_t usec);

    ssize_t read_socket(socket_t sock, void *ptr, size_t size, int flags);

    ssize_t send_socket(socket_t sock, const void *ptr, size_t size, int flags);

    void set_nonblocking(socket_t sock, bool nonblocking);

    bool is_connection_error();

    Error wait_until_socket_is_ready(socket_t sock, time_t sec, time_t usec);

    bool get_remote_ip_and_port(socket_t sock, std::string &ip, int &port);

    bool get_remote_ip_and_port(const struct sockaddr_storage &addr,
                                socklen_t addr_len, std::string &ip,
                                int &port);


    class SocketStream : public Stream {
    public:
        explicit SocketStream(socket_t sock, time_t read_timeout_sec = CPPHTTPLIB_READ_TIMEOUT_SECOND,
                     time_t read_timeout_usec = CPPHTTPLIB_READ_TIMEOUT_USECOND,
                     time_t write_timeout_sec = CPPHTTPLIB_WRITE_TIMEOUT_SECOND,
                     time_t write_timeout_usec = CPPHTTPLIB_WRITE_TIMEOUT_USECOND);
        ~SocketStream() override;

        bool is_readable() const override;
        bool is_writable() const override;

        /*
         * 使用前要先判别，是否可读
         * 返回实际读取的数据; 错误返回-1; 对方端点关闭返回0;
         *
         * 阻塞读取，对端关闭返回负值;
         *
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

        std::vector<char> read_buff_;
        size_t read_buff_off_ = 0;
        size_t read_buff_content_size_ = 0;

        static const size_t read_buff_size_ = 1024 * 4;
    };

    bool write_data(Stream &strm, const char *d, size_t l);


    class stream_line_reader {
    public:
        stream_line_reader(Stream &strm, char *fixed_buffer,
                           size_t fixed_buffer_size);
        const char *ptr() const;        //返回读取内容
        size_t size() const;            //读取的内容字节数
        bool end_with_crlf() const;     //是否以特定字符结尾
        bool getline();                 //读取一行内容

    private:
        void append(char c);

        Stream &strm_;
        char *fixed_buffer_;
        const size_t fixed_buffer_size_;
        size_t fixed_buffer_used_size_ = 0;
        std::string glowable_buffer_;
    };


}

#endif //EXHIBITION_SOCKUTILS_H
