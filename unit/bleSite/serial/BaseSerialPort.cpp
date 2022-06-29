//
// Created by WJG on 2022-5-30.
//

#include <memory.h>
#include "BaseSerialPort.h"

int nM_len = 0;
unsigned char pMsg[ival_comm_buff_size] = {0x00};

BaseSerialPort::BaseSerialPort(){
    memset(mSerialDataBuff, 0, ival_comm_buff_size);
    mBuffLen = 0;
    m_bThread_alive = false;
    is_opened = false;
}

BaseSerialPort::~BaseSerialPort() {
    if(read_thread_!= nullptr && read_thread_->joinable())
        read_thread_->join();
    delete read_thread_;
    read_thread_ = nullptr;
}

bool BaseSerialPort::startReadSerialData(){
    if(!is_opened)
        return false;

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
//    uint8_t* pMsg = nullptr;
//    int32_t nM_len = 0;
    uint8_t* pMsg_start = nullptr;
    uint8_t* pStart = mSerialDataBuff;
    int j = 0;

    pMsg_start = mSerialDataBuff;
    for (int i = 0; i < mBuffLen-1;i++)
    {
        if (pStart[i] == data_start_byte)
        {
            pMsg_start = pStart + i;
            for (j = i+1; j < mBuffLen; j++)
            {
                if (pStart[j] == data_end_byte)
                {
                    nM_len = j - i +1;
//                    pMsg = new uint8_t[nM_len];
//                    if(pMsg != NULL)
                    {
                        memcpy(pMsg, pStart + i, nM_len);
                        if(recv_call_back != nullptr)
                            recv_call_back(pMsg, nM_len);
                    }
                    pMsg_start = pStart + i + nM_len;
                    i += nM_len;

//                    delete pMsg;
//                    pMsg = nullptr;
                    memset(pMsg, 0x00, ival_comm_buff_size);
                    nM_len = 0;
                }
            }
        }
    }
    mBuffLen = (mSerialDataBuff + mBuffLen) - pMsg_start;
    memmove(mSerialDataBuff, pMsg_start, mBuffLen);
    memset(mSerialDataBuff + mBuffLen, 0, ival_comm_buff_size - mBuffLen);
}
