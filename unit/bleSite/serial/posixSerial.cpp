//
// Created by 78472 on 2022/9/8.
//

#include "posixSerial.h"
#include <termio.h>
#include <unistd.h>
#include <fcntl.h>
#include <log/Logging.h>

const static int speed_arr[] = {B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300};
const static int name_arr[] = {115200, 38400, 19200, 9600, 4800, 2400, 1200, 300};

CommonSerial::CommonSerial(string &serial_name)
            : fd_serial(-1), serialName(serial_name){}

CommonSerial::~CommonSerial() {
    closeSerial();
}

bool CommonSerial::openSerial(SerialParamStruct aStruct){
    if(isSerialOpened.load()){
        LOG_RED << "serial already opened...";
        return true;
    }

    fd_serial = open(serialName.c_str(), O_RDWR|O_NOCTTY|O_NDELAY);
    if(fd_serial < 0)
    {
        LOG_RED << serialName << " open error: " << errno;
        return false;
    }

    int res = fcntl(fd_serial, F_SETFL, 0);
    if(res)
    {
        LOG_RED << "fcntl failed";
        close(fd_serial);
        return false;
    }

    if(isatty(STDIN_FILENO)==0)
    {
        LOG_RED << "input is not a terminal devices";
        close(fd_serial);
        return false;
    }

    if(setProperty(aStruct.databits, aStruct.stopbits, aStruct.parity, aStruct.baudrate) == false)
    {
        LOG_RED << "Set Property Error";
        close(fd_serial);
        return false;
    }

    //串口成功打开
    isSerialOpened.store(true);

    return true;
}

bool CommonSerial::closeSerial(){
    if(isSerialOpened.load()){
        isSerialOpened.store(false);
        close(fd_serial);
    }
}

bool CommonSerial::write2Serial(const void *buff, int writeLen) {
    if (buff == nullptr || writeLen <= 0 || fd_serial <= 0) {
        LOG_RED << "buff is null or len <=0 or com port not open.";
        return false;
    }

    ssize_t nWrite = write(fd_serial, buff, writeLen);
    if(nWrite != writeLen)
    {
        LOG_RED << "Write Error!......";
        return false;
    }
    return true;
}


ssize_t CommonSerial::readFromSerial(void *receiveBuff, int readLen) {
    if(receiveBuff == nullptr || readLen <= 0 || fd_serial <= 0){
        LOG_RED << "receiveBuff is nullptr or len <=0 or com port not open.";
        return 0;
    }
   return read(fd_serial, receiveBuff, readLen);
}


bool CommonSerial::setProperty(int databits, int stopbits, int parity, int speed) {
    struct termios options{};
    if (tcgetattr(fd_serial, &options) != 0)
    {
        LOG_RED << "setParity Error, <tcgetattr>....";
        return false;
    }
    options.c_cflag &= ~CSIZE;  //屏蔽字符大小位
    switch (databits) /*设置数据位数*/
    {
        case 7:
            options.c_cflag |= CS7;
            break;
        case 8:
            options.c_cflag |= CS8;
            break;
        default:
            LOG_RED << "setDatabits Error, <unsupported data size>...";
            return false;
    }
    switch (parity)
    {
        case 'n':
        case 'N':
            options.c_cflag &= ~PARENB;    /* Clear parity enable */
            options.c_iflag &= ~INPCK;     /* Enable parity checking */
            break;
        case 'o':
        case 'O':
            options.c_cflag |= (PARODD | PARENB);   /* 设置为奇效验*/
            options.c_iflag |= INPCK;               /* Disnable parity checking */
            break;
        case 'e':
        case 'E':
            options.c_cflag |= PARENB;      /* Enable parity */
            options.c_cflag &= ~PARODD;     /* 转换为偶效验*/
            options.c_iflag |= INPCK;       /* Disnable parity checking */
            break;
        case 'S':
        case 's':  /*as no parity*/
            options.c_cflag &= ~PARENB;
            options.c_cflag &= ~CSTOPB;
            break;
        default:
            LOG_RED << "setParity Error, <unsupported parity>...";
            return false;
    }
    /* 设置停止位*/
    switch (stopbits)
    {
        case 1:
            options.c_cflag &= ~CSTOPB;
            break;
        case 2:
            options.c_cflag |= CSTOPB;
            break;
        default:
            LOG_RED << "setStopbits Error, <unsupported stop bits>...";
            return false;
    }

    /* Set input parity option */
    if (parity != 'n')
        options.c_iflag |= INPCK;

    tcflush(fd_serial, TCIFLUSH);
    options.c_cc[VTIME] = 150;          /* 设置超时15 seconds*/
    options.c_cc[VMIN] = 1;             /* Update the options and do it NOW */

    options.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);    /*Input*/
    options.c_oflag  &= ~OPOST;                             /*Output*/
    options.c_iflag  &= ~(ICRNL | IXON);                    //去掉过滤控制字符
    if(tcsetattr(fd_serial ,TCSANOW, &options) != 0)
    {
        LOG_RED << "set uart Error...";
        return false;
    }

    return setSpeed(speed);
}

bool CommonSerial::setSpeed(int speed) const{
    struct termios Opt{};
    if(tcgetattr(fd_serial, &Opt) != 0){    //获取现有配置信息
        LOG_RED << "uart set speed Error, <tcgetattr>....";
        return false;
    }

    for(int i = 0; i < sizeof(speed_arr)/sizeof(int); i++)
    {
        if(speed == name_arr[i])
        {
            tcflush(fd_serial, TCIOFLUSH);
            cfsetispeed(&Opt, speed_arr[i]);
            cfsetospeed(&Opt, speed_arr[i]);
            int status = tcsetattr(fd_serial, TCSANOW, &Opt);   //保存配置
            if (status != 0)
            {
                LOG_RED << "uart set speed Error, <tcsetattr>....";
                return false;
            }
            tcflush(fd_serial, TCIOFLUSH);
            return true;
        }
    }
}



