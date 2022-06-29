//
// Created by 78472 on 2022/6/14.
//

#include "requestHandler.h"
#include "formatTrans/convert.h"
#include "serial/BLETelinkDongle.h"
#include "formatTrans/bleConfigParam.h"

void downCmdHandler(qlibc::QData& cmdData){
    unsigned char buf[100]{};
    size_t size = bleJsonCmd2Binaray(cmdData, buf, 100);
    for(int i = 0; i < size; i++){
        printf("==>%2X\n", buf[i]);
    }

    shared_ptr<BLETelinkDongle> serial = bleConfigParam::getInstance()->getSerial();
    if(serial != nullptr){
        serial->sendData(buf, static_cast<int>(size));
    }
}