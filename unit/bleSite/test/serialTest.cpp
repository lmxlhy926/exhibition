
#include <iostream>
#include <cstring>
#include "serial/BLETelinkDongle.h"

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


int main() {
    std::cout << "Hello, World!" << std::endl;
    BLETelinkDongle *telinkD = new BLETelinkDongle("COM3");//("\\\\.\\COM10");
    telinkD->initDongle();
    telinkD->regRecvDataProc(fun);
    bool ret = telinkD->startDongle();
    if(!ret)
        printf("====>startDongle() failed \n");

    while(1);

    return 0;
}