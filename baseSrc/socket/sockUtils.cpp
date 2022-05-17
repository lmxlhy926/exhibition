//
// Created by 78472 on 2022/5/2.
//

#include "sockUtils.h"
namespace sockCommon{

    time_t connection_timeout_sec_ = CPPHTTPLIB_CONNECTION_TIMEOUT_SECOND;
    time_t connection_timeout_usec_ = CPPHTTPLIB_CONNECTION_TIMEOUT_USECOND;
    time_t read_timeout_sec_ = CPPHTTPLIB_READ_TIMEOUT_SECOND;
    time_t read_timeout_usec_ = CPPHTTPLIB_READ_TIMEOUT_USECOND;
    time_t write_timeout_sec_ = CPPHTTPLIB_WRITE_TIMEOUT_SECOND;
    time_t write_timeout_usec_ = CPPHTTPLIB_WRITE_TIMEOUT_USECOND;

    socket_t create_socket(const char *ip, int port,
                           const BindOrConnect& bind_or_connect,
                           int socket_flags){
        struct addrinfo hints{};
        struct addrinfo *result;

        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = 0;
        hints.ai_flags = socket_flags;

        auto service = std::to_string(port);

        if (getaddrinfo(ip, service.c_str(), &hints, &result)) {
            return INVALID_SOCKET;
        }

        for (auto rp = result; rp; rp = rp->ai_next) {
            auto sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
            if (sock == INVALID_SOCKET) { continue; }
            if (fcntl(sock, F_SETFD, FD_CLOEXEC) == -1) { continue; }

            {
                int yes = 1;
                setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char *>(&yes),
                           sizeof(yes));
            }

            {
                int yes = 1;
                setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char *>(&yes),
                           sizeof(yes));
            }


            if (rp->ai_family == AF_INET6) {
                int no = 0;
                setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, reinterpret_cast<char *>(&no),
                           sizeof(no));
            }

            // bind or connect
            if (bind_or_connect(sock, *rp)) {
                freeaddrinfo(result);
                return sock;
            }

            close_socket(sock);
        }

        freeaddrinfo(result);
        return INVALID_SOCKET;
    }

    bool connect(socket_t sock2, struct addrinfo &ai){
        Error error;
        set_nonblocking(sock2, true);

        auto ret = connect(sock2, ai.ai_addr, static_cast<socklen_t>(ai.ai_addrlen));
        if (ret < 0) {
            if (is_connection_error()) {
                return false;
            }

            error = wait_until_socket_is_ready(sock2, connection_timeout_sec_,
                                               connection_timeout_usec_);
            if (error != Error::Success) { return false; }
        }

        set_nonblocking(sock2, false);


//        {
//            timeval tv{};
//            tv.tv_sec = static_cast<long>(CPPHTTPLIB_READ_TIMEOUT_SECOND);
//            tv.tv_usec = static_cast<decltype(tv.tv_usec)>(CPPHTTPLIB_READ_TIMEOUT_USECOND);
//            setsockopt(sock2, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(tv));
//        }
//
//        {
//            timeval tv{};
//            tv.tv_sec = static_cast<long>(CPPHTTPLIB_WRITE_TIMEOUT_SECOND);
//            tv.tv_usec = static_cast<decltype(tv.tv_usec)>(CPPHTTPLIB_WRITE_TIMEOUT_USECOND);
//            setsockopt(sock2, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv, sizeof(tv));
//        }

        return true;
    }

    void shutdown_socket(socket_t sock){
        shutdown(sock, SHUT_RDWR);
    }

    void close_socket(socket_t sock){
        close(sock);
    }

    template <typename T> ssize_t handle_EINTR(T fn) {
        ssize_t res = false;
        while (true) {
            res = fn();
            if (res < 0 && errno == EINTR) { continue; }
            break;
        }
        return res;
    }

    //指定时间读取
    ssize_t select_read(socket_t sock, time_t sec, time_t usec){
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(sock, &fds);

        timeval tv{};
        tv.tv_sec = static_cast<long>(sec);
        tv.tv_usec = static_cast<decltype(tv.tv_usec)>(usec);

        return handle_EINTR([&]() {
            return select(static_cast<int>(sock + 1), &fds, nullptr, nullptr, &tv);
        });
    }

    //阻塞读取
    ssize_t select_read(socket_t sock){
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(sock, &fds);


        return handle_EINTR([&]() {
            return select(static_cast<int>(sock + 1), &fds, nullptr, nullptr, nullptr);
        });
    }

    ssize_t select_write(socket_t sock, time_t sec, time_t usec){
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(sock, &fds);

        timeval tv{};
        tv.tv_sec = static_cast<long>(sec);
        tv.tv_usec = static_cast<decltype(tv.tv_usec)>(usec);

        return handle_EINTR([&]() {
            return select(static_cast<int>(sock + 1), nullptr, &fds, nullptr, &tv);
        });
    }

    bool is_socket_alive(socket_t sock){
        const auto val = select_read(sock, 0, 0);
        if (val == 0) {     //没有数据可读，但是通道连接正常的
            return true;
        } else if (val < 0 && errno == EBADF) { //通道连接关闭
            return false;
        }

        //如果可读，则预读取一个数据测试通道（并不会真正的读取）
        char buf[1];
        return read_socket(sock, &buf[0], sizeof(buf), MSG_PEEK) > 0;
    }

    ssize_t read_socket(socket_t sock, void *ptr, size_t size, int flags){
        return handle_EINTR([&]() {
            return recv(sock, ptr, size, flags);
        });
    }

    ssize_t send_socket(socket_t sock, const void *ptr, size_t size, int flags){
        return handle_EINTR([&]() {
            return send(sock, ptr, size, flags);
        });
    }

    void set_nonblocking(socket_t sock, bool nonblocking){
        auto flags = fcntl(sock, F_GETFL, 0);
        fcntl(sock, F_SETFL,
              nonblocking ? (flags | O_NONBLOCK) : (flags & (~O_NONBLOCK)));
    }

    bool is_connection_error(){
        return errno != EINPROGRESS;
    }

    Error wait_until_socket_is_ready(socket_t sock, time_t sec, time_t usec){

        if (sock >= FD_SETSIZE) { return Error::Connection; }

        fd_set fdsr;
        FD_ZERO(&fdsr);
        FD_SET(sock, &fdsr);

        auto fdsw = fdsr;
        auto fdse = fdsr;

        timeval tv{};
        tv.tv_sec = static_cast<long>(sec);
        tv.tv_usec = static_cast<decltype(tv.tv_usec)>(usec);

        auto ret = handle_EINTR([&]() {
            return select(static_cast<int>(sock + 1), &fdsr, &fdsw, &fdse, &tv);
        });

        if (ret == 0) { return Error::ConnectionTimeout; }

        if (ret > 0 && (FD_ISSET(sock, &fdsr) || FD_ISSET(sock, &fdsw))) {
            int error = 0;
            socklen_t len = sizeof(error);
            auto res = getsockopt(sock, SOL_SOCKET, SO_ERROR,
                                  reinterpret_cast<char *>(&error), &len);
            auto successful = res >= 0 && !error;
            return successful ? Error::Success : Error::Connection;
        }
        return Error::Connection;
    }

    bool get_remote_ip_and_port(socket_t sock, std::string &ip, int &port){
        struct sockaddr_storage addr{};
        socklen_t addr_len = sizeof(addr);

        if (!getpeername(sock, reinterpret_cast<struct sockaddr *>(&addr),
                         &addr_len)) {
            return get_ip_and_port(addr, addr_len, ip, port);
        }
        return false;
    }


    bool get_ip_and_port(const struct sockaddr_storage &addr,
                                socklen_t addr_len, std::string &ip,
                                int &port){
        if (addr.ss_family == AF_INET) {
            port = ntohs(reinterpret_cast<const struct sockaddr_in *>(&addr)->sin_port);
        } else if (addr.ss_family == AF_INET6) {
            port =
                    ntohs(reinterpret_cast<const struct sockaddr_in6 *>(&addr)->sin6_port);
        } else {
            return false;
        }

        std::array<char, NI_MAXHOST> ipstr{};
        if (getnameinfo(reinterpret_cast<const struct sockaddr *>(&addr), addr_len,
                        ipstr.data(), static_cast<socklen_t>(ipstr.size()), nullptr,
                        0, NI_NUMERICHOST)) {
            return false;
        }

        ip = ipstr.data();
        return true;
    }


    SocketStream::SocketStream(socket_t sock, time_t read_timeout_sec, time_t read_timeout_usec,
                               time_t write_timeout_sec, time_t write_timeout_usec)
                               : sock_(sock), read_timeout_sec_(read_timeout_sec),
                                 read_timeout_usec_(read_timeout_usec),
                                 write_timeout_sec_(write_timeout_sec),
                                 write_timeout_usec_(write_timeout_usec),
                                 read_buff_(read_buff_size_, 0) {}

    SocketStream::~SocketStream() {}

    bool SocketStream::is_readable() const {
        if(read_buff_off_ < read_buff_content_size_ || select_read(sock_) >  0)
            return true;
        else
            return false;
    }

    bool SocketStream::is_writable() const {
        return select_write(sock_, 0, 0) > 0;
    }

    ssize_t SocketStream::read(char *ptr, size_t size) {
        if(!is_readable())  return -1;

        size = (std::min)(size, static_cast<size_t>((std::numeric_limits<ssize_t>::max)()));
        //当前读取缓存中有数据可以被读取，最多可读取剩余的所有数据
        if (read_buff_off_ < read_buff_content_size_) {
            auto remaining_size = read_buff_content_size_ - read_buff_off_;
            if (size <= remaining_size) {
                memcpy(ptr, read_buff_.data() + read_buff_off_, size);
                read_buff_off_ += size;
                return static_cast<ssize_t>(size);
            } else {
                memcpy(ptr, read_buff_.data() + read_buff_off_, remaining_size);
                read_buff_off_ += remaining_size;
                return static_cast<ssize_t>(remaining_size);
            }
        }

        //当前读取缓存中，无数据可以读取
        read_buff_off_ = 0;
        read_buff_content_size_ = 0;

        if (size < read_buff_size_) {
            auto n = read_socket(sock_, read_buff_.data(), read_buff_size_,
                                 CPPHTTPLIB_RECV_FLAGS);

            if (n <= 0) {   //读取错误
                return n;
            } else if (n <= static_cast<ssize_t>(size)) {   //读取的数据不够
                memcpy(ptr, read_buff_.data(), static_cast<size_t>(n));
                return n;
            } else {    //读取的数据有剩余，返回需要的数据，存储剩余的数据
                memcpy(ptr, read_buff_.data(), size);
                read_buff_off_ = size;
                read_buff_content_size_ = static_cast<size_t>(n);
                return static_cast<ssize_t>(size);
            }
        } else {    //读取大数据时，直接读取，不进行缓存
            return read_socket(sock_, ptr, size, CPPHTTPLIB_RECV_FLAGS);
        }
    }

    ssize_t SocketStream::write(const char *ptr, size_t size) {
        if (!is_writable()) { return -1; }
        return send_socket(sock_, ptr, size, CPPHTTPLIB_SEND_FLAGS);
    }

    void SocketStream::get_remote_ip_and_port(std::string &ip, int &port) const {
        if(!sockCommon::get_remote_ip_and_port(sock_, ip, port)){
            ip = "INVALID_IP";
            port = INVALID_SOCKET;
        }
    }

    socket_t SocketStream::socket() const {
        return sock_;
    }


    bool write_data(Stream &strm, const char *d, size_t l){
        size_t offset = 0;
        while (offset < l) {
            auto length = strm.write(d + offset, l - offset);
            if (length < 0) { return false; }
            offset += static_cast<size_t>(length);
        }
        return true;
    }


    const char *stream_line_reader::ptr() const {
        if (glowable_buffer_.empty()) {
            return fixed_buffer_;
        } else {
            return glowable_buffer_.data();
        }
    }

    size_t stream_line_reader::size() const {
        if (glowable_buffer_.empty()) {
            return fixed_buffer_used_size_;
        } else {
            return glowable_buffer_.size();
        }
    }

    bool stream_line_reader::end_with_crlf() const {
        auto end = ptr() + size();
        return size() >= 1 && end[-1] == '\n';
    }

    bool stream_line_reader::getline() {
        fixed_buffer_used_size_ = 0;
        glowable_buffer_.clear();

        for (size_t i = 0;; i++) {
            char byte;
            auto n = strm_.read(&byte, 1);

            if (n < 0) {    //通道关闭
                return false;
            }else if (n == 0) {
                if (i == 0) {
                    return false;
                } else {
                    break;
                }
            }

            append(byte);

            if (byte == '\n') { break; }
        }

        return true;
    }

    void stream_line_reader::append(char c) {
        if (fixed_buffer_used_size_ < fixed_buffer_size_ - 1) {
            fixed_buffer_[fixed_buffer_used_size_++] = c;
            fixed_buffer_[fixed_buffer_used_size_] = '\0';
        } else {
            if (glowable_buffer_.empty()) {
                assert(fixed_buffer_[fixed_buffer_used_size_] == '\0');
                glowable_buffer_.assign(fixed_buffer_, fixed_buffer_used_size_);
            }
            glowable_buffer_ += c;
        }
    }








}//namespace sockCommon

