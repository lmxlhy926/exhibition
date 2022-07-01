//
// Created by 78472 on 2022/6/14.
//

#include "requestHandler.h"
#include "formatTrans/convert.h"
#include "serial/BLETelinkDongle.h"
#include "formatTrans/bleConfigParam.h"
#include <mutex>

std::mutex sendMutex;

void downCmdHandler(qlibc::QData& cmdData){
    unsigned char buf[100]{};
    size_t size = bleJsonCmd2Binaray(cmdData, buf, 100);
    for(int i = 0; i < size; i++){
        printf("%2X", buf[i]);
    }
    printf("\n");

    shared_ptr<BLETelinkDongle> serial = bleConfigParam::getInstance()->getSerial();
    if(serial != nullptr){
        std::lock_guard<std::mutex> lg(sendMutex);
        if(serial->sendData(buf, static_cast<int>(size))){
            printf("===>send success....\n");
        }
    }
}