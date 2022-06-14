//
// Created by 78472 on 2022/6/14.
//

#ifndef EXHIBITION_CONVERT_H
#define EXHIBITION_CONVERT_H

#include "qlibc/QData.h"

size_t getBleControlBinary(qlibc::QData& data, unsigned char* buf, size_t bufSize);

size_t getBleCommandBinaray(qlibc::QData& data, unsigned char* buf, size_t bufSize);

string binaryCommand2JsonString(unsigned char* buf, size_t bufSize);

#endif //EXHIBITION_CONVERT_H
