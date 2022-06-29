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

typedef struct serial_param{
    int databits;
    int stopbits;
    int parity;
    int baudrate;
}SerialParamStruct;

class BaseSerialPort{
public:
    BaseSerialPort();
    ~BaseSerialPort();
    virtual bool initSerial(std::string serial_name, SerialParamStruct aStruct) = 0;
    bool startReadSerialData();
    bool stopReadSerialData();
    virtual bool writeSerialData(unsigned char *buff, int len) = 0;
    bool regSerialDataProcess(RECV_DATA_CALLBACK fun);
    bool setDataStartEndByte(unsigned char start, unsigned char end);

protected:
    bool m_bThread_alive;
    bool is_opened;
    std::thread *read_thread_ = nullptr;
    std::string serial_port_name;
    unsigned char mSerialDataBuff[ival_comm_buff_size];
    unsigned int  mBuffLen;
    unsigned char data_start_byte = 0x01;
    unsigned char data_end_byte = 0x03;

    virtual void readSerialData() = 0;
    void onSerialDataRead();
    RECV_DATA_CALLBACK recv_call_back = nullptr;
};

#endif //BLE_LIGHT_SITE_BASESERIALPORT_H
