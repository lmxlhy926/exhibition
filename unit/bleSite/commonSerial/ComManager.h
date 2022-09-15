/*
 * ComManager.h
 *
 *  Created on: 2022年5月24日
 *      Author: van
 */

#ifndef SRC_COMMANAGER_H_
#define SRC_COMMANAGER_H_

#include <thread>
#include <vector>
#include <functional>
#include <stdint.h>
#include <queue>
#include <mutex>
#include <semaphore.h>
#include <termios.h>

using namespace std;

using ComRcvDataHandler = std::function<void(std::vector<uint8_t>&)>;

class ComManager {
    static const int PARSE_STATE_HEAD = 1;
    static const int PARSE_STATE_DATA = 2;
    static const int PARSE_STATE_TAIL = 3;

    static int parse_state;
    static std::vector<uint8_t> frame_buffer;

    static int com_fd;

    static string com_path;

    static sem_t rcv_sem;
    static std::queue<std::vector<uint8_t>> rcv_queue;  //接收数据队列
    static std::mutex rcv_queue_mutex;

    static sem_t send_sem;
    static std::queue<std::vector<uint8_t>> send_queue; //发送数据队列
    static std::mutex send_queue_mutex;

    static void comRcvThreadFunc(void);
    static void comHandleThreadFunc(void);
    static void comSendThreadFunc(void);

    std::thread* com_rcv_thread;
    std::thread* com_handle_thread;
    std::thread* com_send_thread;

    static ComRcvDataHandler com_rcv_handler;   //接收数据处理回调

    ComManager();
    static ComManager instance;
public:
    static const speed_t COM_SPEED = B115200;
    static const int RCV_BUF_SIZE = 1000;
    static const int RCV_SEL_TIME_S = 10;

    static const uint8_t FRAME_HEAD;
    static const uint8_t FRAME_CHANGE;
    static const uint8_t FRAME_TAIL;

    static const uint8_t DATA_CHANGE = 0x10;

    static ComManager* getInstance()
    {
        return &instance;
    }

    void init(string com_path);

    //串口打开、关闭、设置
    static int UART_Open();
    static void UART_Close(void);
    static int UART_Set(int speed);

    //串口收发
    static int UART_Recv(uint8_t* rcv_buf, int data_len);
    static int UART_Send(uint8_t* send_buf, int data_len);

    //接收数据，按包处理
    static void handleRcvData(uint8_t* rcv_data, int data_len);
    //包转义后，加入缓存
    static void handleFrame(void);
    //数据放入缓存，等待发送线程发送
    static void sendData(std::vector<uint8_t>& data);

    static void log_hex(string fmt, uint32_t value);
    static void log_vector(string desc, std::vector<uint8_t> list);
    static void log_buff(string desc, uint8_t* buffer, uint16_t buffer_len);

	void setComPath(string comPath) {
		com_path = comPath;
	}

    static void registerComRcvDataHandler(ComRcvDataHandler handler);
};

#endif /* SRC_COMMANAGER_H_ */
