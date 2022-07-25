//
// Created by WJG on 2022-5-30.
//

#ifndef BLE_LIGHT_SITE_BASESERIALPORT_H
#define BLE_LIGHT_SITE_BASESERIALPORT_H

#include <string>
#include <thread>

typedef bool (*RECV_DATA_CALLBACK)(unsigned char *pMsg, int len);

const static uint16_t ival_comm_buff_size = 1024;
const static uint16_t ival_comm_buff_max = 32768;

//设置串口参数结构体
typedef struct serial_param{
    int databits;   //数据位
    int stopbits;   //停止位
    int parity;     //奇偶位
    int baudrate;   //波特率
}SerialParamStruct;

class BaseSerialPort{
public:
    BaseSerialPort();

    ~BaseSerialPort();

    virtual bool initSerial(std::string serial_name, SerialParamStruct aStruct) = 0;

    //开始读取串口数据
    bool startReadSerialData();

    //结束读取串口数据
    bool stopReadSerialData();

    //向串口写数据
    virtual bool writeSerialData(unsigned char *buff, int len) = 0;

    //注册回调函数
    bool regSerialDataProcess(RECV_DATA_CALLBACK fun);

    //设置起始、结束字符
    bool setDataStartEndByte(unsigned char start, unsigned char end);
protected:
    bool m_bThread_alive;
    bool is_opened;                                         //串口是否打开
    std::thread *read_thread_ = nullptr;                    //线程
    std::string serial_port_name;                           //串口名
    unsigned char mSerialDataBuff[ival_comm_buff_size];     //从串口读取到的数据
    unsigned int  mBuffLen;                                 //读取到的数据的长度
    unsigned char data_start_byte = 0x01;                   //起始读字符
    unsigned char data_end_byte = 0x03;                     //终止读字符
    virtual void readSerialData() = 0;
    void onSerialDataRead();                                //读取数据包
    RECV_DATA_CALLBACK recv_call_back = nullptr;            //处理读取数据的回调函数
};

#endif //BLE_LIGHT_SITE_BASESERIALPORT_H
