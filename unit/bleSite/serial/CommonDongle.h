//
// Created by WJG on 2022-5-31.
//

#ifndef BLE_LIGHT_SITE_COMMONDONGLE_H
#define BLE_LIGHT_SITE_COMMONDONGLE_H

#include "BaseSerialPort.h"

#define DATA_BUFF_SIZE 1024

typedef bool (*RECV_DATA_PROC)(unsigned char *data, int len);

class CommonDongle{
public:
    explicit CommonDongle(std::string port_name);
    virtual ~CommonDongle();

    virtual bool initDongle();
    bool startDongle();
    bool stopDongle();
    bool sendData(unsigned char *data, int len);
    bool regRecvDataProc(RECV_DATA_PROC proc_fun);

protected:
    std::string serial_name;
    BaseSerialPort *pSerial = nullptr;
    RECV_DATA_PROC recv_data_proc = nullptr;

};

#endif //BLE_LIGHT_SITE_COMMONDONGLE_H
