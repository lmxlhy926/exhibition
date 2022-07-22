//
// Created by WJG on 2022-5-31.
//

#include <string.h>
#include "CommonDongle.h"
//#if defined(__WIN32__) || defined(__CYGWIN__)
//#include "Win32SerialPort.h"
//#else
#include "PosixSerialPort.h"
//#endif

CommonDongle::CommonDongle(std::string port_name){
    serial_name = port_name;
//#if defined(__WIN32__) || defined(__CYGWIN__)
//    pSerial = new Win32SerialPort();
//#else
    pSerial = new PosixSerialPort();
//#endif

    recv_data_proc = nullptr;
}

CommonDongle::~CommonDongle(){
    stopDongle();
    pSerial = nullptr;
    recv_data_proc = nullptr;
}

bool CommonDongle::initDongle(){
    if(pSerial != nullptr)
    {
        SerialParamStruct _sp;
        _sp.stopbits = 1;
        _sp.databits = 8;
        _sp.parity = 'N';
        _sp.baudrate = 9600;//115200;

        return pSerial->initSerial(serial_name, _sp);
    }

    return false;
}

bool CommonDongle::startDongle(){
    if(recv_data_proc == nullptr)
        return false;

    if(pSerial != nullptr)
        return pSerial->startReadSerialData();

    return false;
}

bool CommonDongle::stopDongle(){
    if(pSerial != nullptr)
        return pSerial->stopReadSerialData();

    return false;
}

bool CommonDongle::sendData(unsigned char *data, int len){
    if(data == nullptr)
        return false;

    if(len <= DATA_BUFF_SIZE)
        return pSerial->writeSerialData(data, len);

    return false;
}

bool CommonDongle::regRecvDataProc(RECV_DATA_PROC proc_fun){
    if(proc_fun != nullptr)
    {
        recv_data_proc = proc_fun;
        pSerial->regSerialDataProcess(proc_fun);
        return true;
    }

    return false;
}
