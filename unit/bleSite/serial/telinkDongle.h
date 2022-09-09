//
// Created by 78472 on 2022/9/8.
//

#ifndef EXHIBITION_TELINKDONGLE_H
#define EXHIBITION_TELINKDONGLE_H

#include "posixSerial.h"
#include <thread>
#include <functional>

#define MaxMessageSize (4 * 1024)

//包消息处理
using PackageMsgHandleFuncType = std::function<bool(unsigned char *pMsg, int len)>;

class TelinkDongle {
private:
    std::thread *read_thread_ = nullptr;                        //线程
    unsigned char mSerialDataBuff[MaxMessageSize + 1]{};        //从串口读取到的数据
    unsigned int  mBuffLen;                                     //缓冲区中有效数据
    unsigned char data_start_byte = 0x01;                       //包起始字符
    unsigned char data_end_byte = 0x03;                         //包终止字符
    PackageMsgHandleFuncType packageMsgHandledFunc = nullptr;   //处理读取数据的回调函数
    PosixSerial posixSerial;                                    //实际的串口操作对象

public:
    explicit TelinkDongle(std::string& serial_name, SerialParamStruct& aStruct);

    ~TelinkDongle();

    //注册包消息处理函数
    void registerPkgMsgFunc(PackageMsgHandleFuncType fun);

    //开始读取并处理从串口读取到的数据
    bool startReadAndHandleSerial();

    //向串口写数据
    bool write2Seria(unsigned char *buff, int len);

    //关闭串口
    void closeSerial();

private:
    void readFromSerialAndHandle();    //从串口读取数据并处理

    void onSerialDataRead();           //将数据拆分为包，并调用包消息处理函数进行处理
};


#endif //EXHIBITION_TELINKDONGLE_H
