/*
 * main.cpp
 *
 *  Created on: 2022年5月24日
 *      Author: van
 */
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <thread>
#include <vector>
#include <functional>
#include <stdint.h>
#include <queue>
#include <mutex>
#include <semaphore.h>
#include <termios.h>

#include "ComManager.h"

using namespace std;

// 处理接收数据回调函数
void handle_rcv_data(std::vector<uint8_t>& rcv_data) {
	printf("rcv_data: ");
	for (int i = 0; i < rcv_data.size(); ++i) {
		printf("%02X ", rcv_data[i]);
	}
	printf("\n");
}

// 发送数据示例
void send_data(void) {
	std::vector<uint8_t> send_data;

	send_data.push_back(0xE9);
	send_data.push_back(0xFF);
	send_data.push_back(0x00);

	// 发送串口数据
	ComManager::getInstance()->sendData(send_data);
}

int main(int argc, char *argv[]) {
	// string com_port = "/dev/ttyS1";
	string com_port = "/dev/ttyUSB0";

	// 注册接收数据处理回调函数
	ComManager::getInstance()->registerComRcvDataHandler(handle_rcv_data);
	// 初始化并开启串口收发
	ComManager::getInstance()->init(com_port);

	// 等待发送线程初始化 ！！！！
	sleep(1);

	// 发送数据示例
	send_data();

	while (1) {
		sleep(60);
	}

	return 0;
}
