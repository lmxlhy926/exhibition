//
// Created by WJG on 2022-5-30.
//

#ifndef BLE_LIGHT_SITE_WIN32SERIALPORT_H
#define BLE_LIGHT_SITE_WIN32SERIALPORT_H

#include <windows.h>
#include "BaseSerialPort.h"

class Win32SerialPort : public BaseSerialPort {

public:
    Win32SerialPort();
    ~Win32SerialPort();

    bool initSerial(std::string serial_name, SerialParamStruct aStruct) override;
    bool writeSerialData(unsigned char *buff, int len) override;


protected:
    void readSerialData() override;

private:
    HANDLE m_hCom;
    OVERLAPPED m_ov;
    COMMTIMEOUTS m_CommTimeouts;
    DCB m_dcb;
    DWORD m_dwComm_events;

    HANDLE m_hThr_shut_event;
    HANDLE m_hEvent_array[2];
    void procSerialData();
};

#endif //BLE_LIGHT_SITE_WIN32SERIALPORT_H
