/*
 * ComManager.cpp
 *
 *  Created on: 2022年5月24日
 *      Author: van
 */
#include <errno.h>
#include <fcntl.h>
#include <queue>
#include <semaphore.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <thread>
#include <unistd.h>
#include <vector>
#include <mutex>

// #include "easylogging++.h"

#include "ComManager.h"

using namespace std;

int ComManager::com_fd = -1;

string ComManager::com_path = "";

sem_t ComManager::rcv_sem;
std::queue<std::vector<uint8_t>> ComManager::rcv_queue;
std::mutex ComManager::rcv_queue_mutex;

sem_t ComManager::send_sem;
std::queue<std::vector<uint8_t>> ComManager::send_queue;
std::mutex ComManager::send_queue_mutex;

ComRcvDataHandler ComManager::com_rcv_handler;

int ComManager::parse_state = PARSE_STATE_HEAD;
std::vector<uint8_t> ComManager::frame_buffer;

const uint8_t ComManager::FRAME_HEAD = 0x01;
const uint8_t ComManager::FRAME_CHANGE = 0x02;
const uint8_t ComManager::FRAME_TAIL = 0x03;

ComManager ComManager::instance;

void ComManager::comRcvThreadFunc(void)
{
    int ret;
    uint8_t rcv_buf[RCV_BUF_SIZE];

    ret = UART_Open();
    if (ret == -1) {
        // printf("open error\n");
        // LOG(ERROR) << "open error";
        exit(1);
    }

    ret = UART_Set(COM_SPEED);
    if (-1 == ret) {
        // printf("Set Port Error\n");
        // LOG(ERROR) << "Set Port Error";
        exit(1);
    }

    while (true) {
        ret = UART_Recv(rcv_buf, RCV_BUF_SIZE);
        if (ret > 0) {
            handleRcvData(rcv_buf, ret);
        } else {
            // printf("cannot receive data\n");
        }
    }
}

void ComManager::registerComRcvDataHandler(ComRcvDataHandler handler) {
    com_rcv_handler = handler;
}

void ComManager::comHandleThreadFunc(void) {
    while (true) {
        sem_wait(&rcv_sem);

        rcv_queue_mutex.lock();
        std::vector<uint8_t> ok_frame = rcv_queue.front();

        com_rcv_handler(ok_frame);

        // 出队
        rcv_queue.pop();
        rcv_queue_mutex.unlock();
    }
}

int ComManager::UART_Open()
{
    com_fd = open(com_path.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    if (com_fd == -1) {
        // perror("Can't Open Serial Port");
        // LOG(ERROR) << "Can't Open Serial Port" << com_path;
        return -1;
    }

    //判断串口的状态是否为阻塞状态
    if (fcntl(com_fd, F_SETFL, 0) < 0) {
        // printf("fcntl failed!\n");
        // LOG(ERROR) << "fcntl failed!";
        return -1;
    } else {
        // printf("fcntl=%d\n",fcntl(com_fd, F_SETFL,0));
    }

    //测试是否为终端设备
    if (0 == isatty(STDIN_FILENO)) {
        // printf("standard input is not a terminal device\n");
        // LOG(ERROR) << "standard input is not a terminal device";
        return -1;
    }

    return 0;
}

void ComManager::UART_Close(void)
{
    close(com_fd);
}

int ComManager::UART_Set(int speed)
{
    // Create new termios struc, we call it 'tty' for convention
	struct termios tty;

    // Read in existing settings, and handle any error
    if (tcgetattr(com_fd, &tty) != 0) {
        // printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
        // LOG(ERROR) << "Error" << errno << "from tcgetattr:" << strerror(errno);
    }

    // //设置数据位数 8位数据位
    // tty.c_cflag &= ~CSIZE;
    // tty.c_cflag |= CS8;
    // //校验位 无校验位
    // tty.c_cflag &= ~PARENB;
    // tty.c_iflag &= ~INPCK;
    // //设置停止位  1位停止位
    // tty.c_cflag &= ~CSTOPB;
    // tty.c_cflag |= CLOCAL | CREAD;
    // tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    // tty.c_oflag &= ~OPOST;
    // tty.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

    tty.c_cflag |= (CLOCAL | CREAD); //(本地连接（不改变端口所有者)|接收使能)
    tty.c_cflag &= ~CSIZE; //屏蔽字符大小位
    tty.c_cflag &= ~CRTSCTS; //不使用流控制
    tty.c_cflag |= CS8; // 8个数据位
    tty.c_cflag &= ~CSTOPB; //~1位停止位
    tty.c_iflag |= IGNPAR; //忽略奇偶校验错误
    tty.c_iflag &= ~(ICRNL | IXON); //~(将CR映射到NL|启动出口硬件流控)
    tty.c_oflag = 0; //输出模式标志
    tty.c_lflag = 0; //本地模式标志

    // tty.c_cc[VTIME] = 10; // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
    // tty.c_cc[VMIN] = 0;

    // Set in/out baud rate to be 9600
    cfsetispeed(&tty, COM_SPEED);
    cfsetospeed(&tty, COM_SPEED);

    // Save tty settings, also checking for error
	if (tcsetattr(com_fd, TCSANOW, &tty) != 0) {
		// printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
        // LOG(ERROR) << "Error" <<  errno << "from tcsetattr:" << strerror(errno);
	}

    return 0;
}

int ComManager::UART_Recv(uint8_t* rcv_buf, int data_len)
{
    uint16_t len;
    int fs_sel;
    fd_set fs_read;

    struct timeval time;

    FD_ZERO(&fs_read);
    FD_SET(com_fd, &fs_read);

    time.tv_sec = RCV_SEL_TIME_S;
    time.tv_usec = 0;

    //使用select实现串口的多路通信
    fs_sel = select(com_fd + 1, &fs_read, NULL, NULL, &time);
    if (fs_sel) {
        len = read(com_fd, rcv_buf, data_len);
        return len;
    } else {
        return -1;
    }
}

int ComManager::UART_Send(uint8_t* send_buf, int data_len)
{
    int ret;

    ret = write(com_fd, send_buf, data_len);
    if (data_len == ret) {
        return ret;
    } else {
        tcflush(com_fd, TCOFLUSH);
        return -1;
    }
}

ComManager::ComManager() {
}

void ComManager::handleRcvData(uint8_t* rcv_data, int data_len)
{
    int i;

    // printf("raw rcv: ");
    // for (i = 0; i < data_len; ++i) {
    //     printf("%02X ", rcv_data[i]);
    // }
    // printf("\n");

    for (i = 0; i < data_len; ++i) {
        // printf("%02X\n", rcv_data[i]);

        if (parse_state == PARSE_STATE_HEAD) {
            if (rcv_data[i] == FRAME_HEAD) {
                frame_buffer.push_back(rcv_data[i]);
                parse_state = PARSE_STATE_DATA;
            }
            continue;
        }

        if (parse_state == PARSE_STATE_DATA) {
            if (rcv_data[i] == FRAME_TAIL) {
                frame_buffer.push_back(rcv_data[i]);
                handleFrame();
            } else {
                frame_buffer.push_back(rcv_data[i]);
            }
            continue;
        }
    }
}

void ComManager::comSendThreadFunc(void)
{
    while (true) {
        sem_wait(&send_sem);

        send_queue_mutex.lock();
        std::vector<uint8_t> data = send_queue.front();

        uint8_t send_buff[data.size()];
        std::copy(data.begin(), data.end(), send_buff);

        // log_buff("send com data:", send_buff, data.size());

        // 出队
        send_queue.pop();

        int ret = UART_Send(send_buff, data.size());
        if (ret < 0) {
            // printf("comSendThreadFunc send error.\n");
            // LOG(ERROR) << "comSendThreadFunc send error.";
        }
        send_queue_mutex.unlock();

        usleep(100000);
    }
}

void ComManager::handleFrame(void)
{
    log_vector("rcv frame_buffer:", frame_buffer);

    std::vector<uint8_t> ok_frame;
    bool need_change = false;

    for (uint8_t& data : frame_buffer) {
        if (data == PARSE_STATE_HEAD) {
            continue;
        }

        if (data == FRAME_CHANGE) {
            need_change = true;
            continue;
        }

        if (data == PARSE_STATE_TAIL) {
            continue;
        }

        if (need_change) {
            need_change = false;
            ok_frame.push_back(DATA_CHANGE ^ data);
        } else {
            ok_frame.push_back(data);
        }
    }
    log_vector("rcv ok_frame:", ok_frame);

    if (ok_frame.size() < 6) {
        parse_state = PARSE_STATE_HEAD;
        frame_buffer.clear();

        return;
    }

    parse_state = PARSE_STATE_HEAD;
    frame_buffer.clear();

    // uint8_t in_check_sum = ok_frame[0];

    // size_t i;
    // for (i = 1; i < 4; ++i) {
    //     in_check_sum = in_check_sum ^ ok_frame[i];
    // }

    // for (i = 5; i < ok_frame.size(); ++i) {
    //     in_check_sum = in_check_sum ^ ok_frame[i];
    // }

    // log_hex("in_check_sum: %02X", in_check_sum);

    // log_hex("check_sum: %02X", ok_frame[4]);

    // if (in_check_sum != ok_frame[4]) {
    //     return;
    // }

    // 入队
    rcv_queue_mutex.lock();
    rcv_queue.push(ok_frame);
    rcv_queue_mutex.unlock();

    sem_post(&rcv_sem);
}

void ComManager::sendData(std::vector<uint8_t>& send_data)
{
    // vector<uint8_t> ok_frame;

    // ok_frame.push_back(ComManager::FRAME_HEAD);

    // for (uint8_t data : send_data) {
    //     if (data < ComManager::DATA_CHANGE) {
    //         ok_frame.push_back(ComManager::FRAME_CHANGE);
    //         ok_frame.push_back(data ^ ComManager::DATA_CHANGE);
    //     } else {
    //         ok_frame.push_back(data);
    //     }
    // }

    // ok_frame.push_back(ComManager::FRAME_TAIL);

    // printf("send_ok_frame: ");
    // for (uint8_t& item : ok_frame) {
    //     printf("%02X ", item);
    // }
    // printf("\n");
    // log_vector("send_ok_frame:", ok_frame);

    printf("send_data: ");
    for (uint8_t& item : send_data) {
        printf("%02X ", item);
    }
    printf("\n");

    send_queue_mutex.lock();
    send_queue.push(send_data);
    send_queue_mutex.unlock();

    sem_post(&send_sem);
}

void ComManager::log_hex(string fmt, uint32_t value) {
    int buff_len = fmt.length() + 8 + 100;
    char* display_buff = new char[buff_len];
    display_buff[0] = 0;

    sprintf(display_buff, fmt.c_str(), value);

    // LOG(INFO) << display_buff;
    printf("%s\n", display_buff);

    delete[] display_buff;
}

void ComManager::log_vector(string desc, std::vector<uint8_t> list) {
    int buff_len = desc.length() + list.size() * 3 + 100;
    char* display_buff = new char[buff_len];
    display_buff[0] = 0;
    
    for (uint8_t& item : list) {
        sprintf(display_buff, "%s%02X ", display_buff, item);
    }

    // LOG(INFO) << desc << display_buff;
    // printf("%s\n", display_buff);

    delete[] display_buff;
}

void ComManager::log_buff(string desc, uint8_t *buffer, uint16_t buffer_len) {
    int buff_len = desc.length() + buffer_len * 3 + 100;
    char* display_buff = new char[buff_len];
    display_buff[0] = 0;
    
    for (int i = 0; i < buffer_len; ++i) {
        sprintf(display_buff, "%s%02X ", display_buff, buffer[i]);
    }

    // LOG(INFO) << desc << display_buff;
    printf("%s\n", display_buff);

    delete[] display_buff;
}

void ComManager::init(string com_path) {
    ComManager::com_path = com_path;
    
	// 初始化信号量
    sem_init(&rcv_sem, 0, 0);
	sem_init(&send_sem, 0, 0);

	// 创建线程
	com_rcv_thread = new std::thread(comRcvThreadFunc);

    com_handle_thread = new std::thread(comHandleThreadFunc);

	com_send_thread = new std::thread(comSendThreadFunc);
}

