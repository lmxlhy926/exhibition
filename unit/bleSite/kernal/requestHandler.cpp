//
// Created by 78472 on 2022/6/14.
//

#include "requestHandler.h"
#include "../formatTrans//convert.h"

void downCmdHandler(qlibc::QData& cmdData){
    unsigned char buf[100]{};
    size_t size = bleJsonCmd2Binaray(cmdData, buf, 100);
    for(int i = 0; i < size; i++){
        printf("==>%2x\n", buf[i]);
    }
}