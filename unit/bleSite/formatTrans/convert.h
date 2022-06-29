//
// Created by 78472 on 2022/6/14.
//

#ifndef EXHIBITION_CONVERT_H
#define EXHIBITION_CONVERT_H

#include "qlibc/QData.h"

/**
 * 依据下发的请求命令获得二进制控制命令
 * @param data      控制命令
 * @param buf       二进制命令数组
 * @param bufSize   数组容量
 * @return          二进制命令长度
 */
size_t bleJsonCmd2Binaray(qlibc::QData& data, unsigned char* buf, size_t bufSize);

string binaryCommand2JsonStringEvent(unsigned char* buf, size_t bufSize);

#endif //EXHIBITION_CONVERT_H
