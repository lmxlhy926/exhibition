//
// Created by 78472 on 2022/9/8.
//

#include "telinkDongle.h"
#include <cstring>
#include <log/Logging.h>

unsigned char packageMessage2Handled[MaxMessageSize] = {0x00};

TelinkDongle::TelinkDongle(std::string& serial_name, SerialParamStruct& aStruct)
                    : posixSerial(serial_name, aStruct){
    memset(mSerialDataBuff, 0, MaxMessageSize);
    mBuffLen = 0;
}

/**
 * 关闭串口，退出读取线程
 */
TelinkDongle::~TelinkDongle() {
    closeSerial();
    if(read_thread_!= nullptr && read_thread_->joinable())
        read_thread_->join();   //阻塞回收线程
    delete read_thread_;        //线程析构
    read_thread_ = nullptr;
}

void TelinkDongle::registerPkgMsgFunc(PackageMsgHandleFuncType fun) {
    packageMsgHandledFunc = std::move(fun);
}

bool TelinkDongle::startReadAndHandleSerial() {
    if(!posixSerial.isOpened()){
        LOG_RED << posixSerial.getSerialName() << " is not opened....";
        return false;
    }

    //创建线程，读取数据
    read_thread_ = new std::thread([this]() {
        readFromSerialAndHandle();
    });

    return true;
}

bool TelinkDongle::write2Seria(unsigned char *buff, int len) {
    return posixSerial.writeSerialData(buff, len);
}

void TelinkDongle::closeSerial() {
    posixSerial.closeSerial();
}

void TelinkDongle::readFromSerialAndHandle() {
    if(posixSerial.isOpened())
    {
        fd_set readSet, allset;
        FD_ZERO(&allset);
        FD_SET(posixSerial.getSerialFd(), &allset);

        while (true)
        {
            readSet = allset;
            if(posixSerial.isOpened()){
                select(posixSerial.getSerialFd() + 1, &readSet, nullptr, nullptr, nullptr);

                int BytesToRead = MaxMessageSize - mBuffLen;
                BytesToRead = static_cast<int>(posixSerial.readSerialData(mSerialDataBuff+mBuffLen, BytesToRead));
                if (BytesToRead > 0)    //读取到数据则进行处理
                {
                    mBuffLen += BytesToRead;
                    onSerialDataRead();
                }
            }else{
                break;
            }
        }
    }
}

/**
 *  1. 定位到包头，如果找不到包头，丢弃所有数据
 *  2. 定位到包尾，如果找不到包尾，将包头至末端的数据左移至缓冲区开头
 *  3. 处理提取到的包数据
 *  4. 读取缓冲区剩余容量的数据到缓冲区，重复上述操作
 */
void TelinkDongle::onSerialDataRead() {
    uint8_t* possiblePackageStart = mSerialDataBuff;
    uint8_t* receiveBufBegin = mSerialDataBuff;

    for (unsigned int i = 0; i < mBuffLen; i++)
    {
        //定位到包消息头
        if (receiveBufBegin[i] == data_start_byte)
        {
            possiblePackageStart = receiveBufBegin + i;

            //寻找包尾
            for (unsigned int j = i+1; j < mBuffLen; j++)
            {
                //定位到包尾
                if (receiveBufBegin[j] == data_end_byte)
                {
                    int packageLength = j - i +1;      //包长度
                    memcpy(packageMessage2Handled, possiblePackageStart, packageLength);
                    if(packageMsgHandledFunc != nullptr)
                        packageMsgHandledFunc(packageMessage2Handled, packageLength);

                    //移到已处理包的下一个位置
                    possiblePackageStart += packageLength;
                    i += packageLength - 1;


                    //清空存放包数据的数组
                    memset(packageMessage2Handled, 0x00, MaxMessageSize);
                    break;
                }

                //没有找到结束标志,退出查找
                if(mBuffLen -1 == j){
                    i = mBuffLen - 1;
                }
            }

        }else{  //未定位到开头，则向前移动
            possiblePackageStart += 1;
        }
    }

    //保留剩余的未处理数据到缓冲区头部
    mBuffLen = (mSerialDataBuff + mBuffLen) - possiblePackageStart;
    memmove(mSerialDataBuff, possiblePackageStart, mBuffLen);
    memset(mSerialDataBuff + mBuffLen, 0, MaxMessageSize - mBuffLen);
}


