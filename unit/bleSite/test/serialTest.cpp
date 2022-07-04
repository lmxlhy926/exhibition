
#include <iostream>
#include <cstring>
#include "serial/BLETelinkDongle.h"
#include <iomanip>
#include <sstream>

unsigned char tmp_buf[256] = {0x00};


bool fun(unsigned char *data, int len){
    memset(tmp_buf, 0x00, 256);
//    printf("rev data len is %d,data is: ",len);
//    for(int i=0; i<len; i++)
//        printf("%02X ", data[i]);
//    printf("\n*****************************\n");
    int j = 0;
    int end_index = len-1;
    if(data[0]== 0x01)
    {
        for(int i=1; i<end_index; i++)
        {
            if(data[i]==0x02)
            {
                tmp_buf[j] = data[i+1]&0x0F;
                i = i+1;
            }
            else
                tmp_buf[j] = data[i];
            j = j+1;
        }
    }

    printf("tras buf len is %d,buf is: ",j);
    for(int i=0; i<j; i++)
        printf("%02X ", tmp_buf[i]);
    printf("\n######################\n");
    return true;
}

//关灯格式
void off(){
    unsigned originBuf[100] = {0x09, 0x00, 0x91, 0x81, 0x02, 0x00, 0x01, 0x00, 0x82, 0x04, 0x00};
    int originBufLength = 11;

    int index = 0;
    unsigned encryptBuf[100];
    for( int i = 0; i < originBufLength; i++){
        if(originBuf[i] < 0x10){
            encryptBuf[index++] = 0x02;
            encryptBuf[index++] = originBuf[i] ^ 0x10;
        }else{
            encryptBuf[index++] = originBuf[i];
        }
    }

    for( int i = 0; i < index; i++){
        std::cout << std::setw(2) << std::setfill('0') << std::hex << encryptBuf[i] << " ";
    }
    std::cout << std::endl;
}

void printfTest(){
    unsigned char buf[] = {0x01, 0x02, 0x1A, 0x02, 0x10, 0x91, 0x81, 0x02, 0x10, 0x02, 0x12, 0x02, 0x10, 0x02, 0x11, 0x02, 0x10, 0x82,
                      0x02, 0x14, 0x02, 0x10, 0x03};
    int bufSize = 23;

    std::stringstream ss;
    for( int i = 0; i < bufSize; i++){
        ss << std::setw(2) << std::setfill('0') << std::hex << buf[i] << " ";
    }
    ss << std::endl;

    std::cout << ss.str();
}



int main() {
//    std::cout << "Hello, World!" << std::endl;
//    BLETelinkDongle *telinkD = new BLETelinkDongle("COM3");//("\\\\.\\COM10");
//    telinkD->initDongle();
//    telinkD->regRecvDataProc(fun);
//    bool ret = telinkD->startDongle();
//    if(!ret)
//        printf("====>startDongle() failed \n");

//    printfTest();

    unsigned char buf[] = {0x01, 0x02, 0x1A, 0x02, 0x10, 0x91, 0x81, 0x02, 0x10, 0x02, 0x12, 0x02, 0x10, 0x02, 0x11, 0x02, 0x10, 0x82,
                           0x02, 0x14, 0x02, 0x10, 0x03};
    std::cout << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(buf[0]) << std::endl;
    std::cout << std::setw(2) << std::setfill('0') << std::hex << 0x01 << std::endl;


    return 0;
}