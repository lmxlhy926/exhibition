//
// Created by WJG on 2022-5-30.
//

#include <memory.h>
#include "BaseSerialPort.h"

unsigned char packageMessage2Handled[MaxMessageSize] = {0x00};

BaseSerialPort::BaseSerialPort(){
    memset(mSerialDataBuff, 0, MaxMessageSize);
    mBuffLen = 0;
    m_bThread_alive = false;
    is_opened = false;
}

BaseSerialPort::~BaseSerialPort() {
    if(read_thread_!= nullptr && read_thread_->joinable())
        read_thread_->join();   //阻塞回收线程
    delete read_thread_;        //线程析构
    read_thread_ = nullptr;
}

bool BaseSerialPort::startReadSerialData(){
    if(!is_opened)
        return false;

    //创建线程，读取数据
    read_thread_ = new std::thread([this]() {
        readSerialData();
    });

    read_thread_->detach();

    return true;
}

bool BaseSerialPort::stopReadSerialData(){
    m_bThread_alive = false;
    is_opened = true;
    if(read_thread_!= nullptr && read_thread_->joinable())
        read_thread_->join();
    delete read_thread_;
    read_thread_ = nullptr;
    return true;
}

bool BaseSerialPort::regSerialDataProcess(RECV_DATA_CALLBACK fun){
    if(fun != nullptr)
    {
        recv_call_back = fun;
        return true;
    }

    return false;
}

bool BaseSerialPort::setDataStartEndByte(unsigned char start, unsigned char end){
    if(start >=0)
        data_start_byte = start;

    if(end >= 0)
        data_end_byte = end;

    return true;
}

void BaseSerialPort::onSerialDataRead() {
    int32_t packageLength = 0;
    uint8_t* possiblePackageStart = mSerialDataBuff + mBuffLen;
    uint8_t* receiveBufBegin = mSerialDataBuff;
    int j = 0;

    possiblePackageStart = mSerialDataBuff;
    for (unsigned int i = 0; i < mBuffLen;i++)
    {
        //定位到包消息头
        if (receiveBufBegin[i] == data_start_byte)
        {
            possiblePackageStart = receiveBufBegin + i;

            //寻找包尾
            for (j = i+1; j < mBuffLen; j++)
            {
                //定位到包尾
                if (receiveBufBegin[j] == data_end_byte)
                {
                    packageLength = j - i +1;      //包长度
                    memcpy(packageMessage2Handled, receiveBufBegin + i, packageLength);
                    if(recv_call_back != nullptr)
                        recv_call_back(packageMessage2Handled, packageLength);

                    //移到已处理包的下一个位置
                    possiblePackageStart = receiveBufBegin + i + packageLength;
                    i += packageLength - 1;


                    //清空存放包数据的数组
                    memset(packageMessage2Handled, 0x00, MaxMessageSize);
                    break;
                }

                //没有找到结束标志
                if(mBuffLen -1 == j){
                    i = mBuffLen - 1;
                }
            }
        }
    }

    //保留剩余的未处理数据到缓冲区头部
    mBuffLen = (mSerialDataBuff + mBuffLen) - possiblePackageStart;
    memmove(mSerialDataBuff, possiblePackageStart, mBuffLen);
    memset(mSerialDataBuff + mBuffLen, 0, MaxMessageSize - mBuffLen);
}
