//
// Created by 78472 on 2022/6/15.
//

#ifndef EXHIBITION_DOWNCMD_H
#define EXHIBITION_DOWNCMD_H

#include <string>
#include "qlibc/QData.h"
#include "JsonCmd2Binary.h"

using namespace std;

/**
 * 依据下发的请求命令获得二进制控制命令
 * @param data      控制命令
 * @param buf       二进制命令数组
 * @param bufSize   数组容量
 * @return          二进制命令长度
 */
size_t bleJsonCmd2Binaray(qlibc::QData& data, unsigned char* buf, size_t bufSize);



class LightScanAddDel : public JsonCmd2Binary{
private:
    string pseudoCommand;
    string device_id;
public:
    explicit LightScanAddDel(qlibc::QData& data){ init(data); }

    void init(qlibc::QData& data);

    size_t getBinary(unsigned char* buf, size_t bufSize) override;
};


class LightOnOff : public JsonCmd2Binary{
private:
    string pseudoCommand;
    string address;
public:
    explicit LightOnOff(qlibc::QData& data){ init(data); }

    void init(qlibc::QData& data);

    size_t getBinary(unsigned char* buf, size_t bufSize) override;
};



#endif //EXHIBITION_DOWNCMD_H
