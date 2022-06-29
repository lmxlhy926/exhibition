#include <iostream>
#include "BLETelinkDongle.h"
#include "log_tool.h"
#include "httplib.h"

//unsigned char test[6] = {0x01, 0xE9, 0xFF, 0x02, 0x10, 0x03};
unsigned char test[3] = {0xE9, 0xFF, 0x00};
unsigned char tmp_buf[256] = {0x00};

httplib::Server svr;
BLETelinkDongle *telinkD;

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

void on_process(const httplib::Request &req, httplib::Response &res){
    unsigned char cmd_data[14] = {0xE8,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x82,0x02,0x01,0x00};
    telinkD->sendData(cmd_data, 14);
    res.set_content("ok", "text/plain");
    res.status = 200;
}
void off_process(const httplib::Request &req, httplib::Response &res){
    unsigned char cmd_data[14] = {0xE8,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x82,0x02,0x00,0x00};
    telinkD->sendData(cmd_data, 14);
    res.set_content("ok", "text/plain");
    res.status = 200;
}

bool startHttpService()
{
    bool ret;

    svr.Post("/turnOn", on_process);
    svr.Post("/turnOff", off_process);

    ret = svr.listen("0.0.0.0", 60000);
    return ret;
}

int main() {
    std::cout << "Hello, World!" << std::endl;

    std::thread httpservice(startHttpService);
    httpservice.detach();

    telinkD = new BLETelinkDongle("COM10");//("\\\\.\\COM10");
    ipp_LogD("fdsafajfalf\n");
    telinkD->initDongle();
    telinkD->regRecvDataProc(fun);
    bool ret = telinkD->startDongle();
    if(!ret)
        printf("startDongle() \n");
//    telinkD->sendData(test, 3);

    while(1);

    return 0;
}