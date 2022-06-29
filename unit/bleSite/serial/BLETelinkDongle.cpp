//
// Created by WJG on 2022-5-31.
//

#include "BLETelinkDongle.h"

BLETelinkDongle::BLETelinkDongle(std::string _name):CommonDongle(_name){
}


bool BLETelinkDongle::initDongle(){
    if(pSerial != nullptr)
    {
        SerialParamStruct _sp;
        _sp.stopbits = 1;
        _sp.databits = 8;
        _sp.parity = 'N';
        _sp.baudrate = 115200;

        pSerial->setDataStartEndByte( 0x01, 0x03);

        return pSerial->initSerial(serial_name, _sp);
    }

    return false;
}
