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

PosixSerial::PosixSerial(string &serial_name, SerialParamStruct aStruct)
            : fd_serial(-1), serialName(serial_name){
    initSerial(serial_name, aStruct);
}

PosixSerial::~PosixSerial() {
    closeSerial();
}

bool PosixSerial::writeSerialData(unsigned char *buff, int writeLen) {
    std::lock_guard<std::mutex> lg(readMutex);
    if(!isSerialOpened.load()){
        LOG_RED << serialName << " is no opened....";
        return false;
    }

    int8_t leftTry = ival_comm_write_try_times_;
    size_t nLeft = writeLen;

    if (buff == nullptr || writeLen <= 0 || fd_serial <= 0) {
        LOG_RED << "buf is null or len <=0 or com port not open.";
        return false;
    }

    uint8_t start[128] = {0x00};
    memcpy(start, buff, writeLen);
    uint8_t * ptr = start;
    while (leftTry--)
    {
        ssize_t ret = write(fd_serial, ptr, nLeft);
        if(ret <= 0)
        {
            LOG_RED << "Write Error!......";
            closeSerial();
            return false;
        }

        nLeft -= ret;

        if(nLeft <= 0)
        {
            return true;
        }
        ptr += ret;
    }

    return true;
}


ssize_t PosixSerial::readSerialData(unsigned char *receiveBuff, int readLen) {
    std::lock_guard<std::mutex> lg(writeMutex);
    if(!isSerialOpened.load()){
        LOG_RED << serialName << " is not opened...";
        return -1;
    }

    ssize_t readCount =  read(fd_serial, receiveBuff, readLen);
    if(readCount < 0){
        closeSerial();
        return -1;
    }else{
        return readCount;
    }
}

void PosixSerial::closeSerial(){
    if(isSerialOpened.load()){
        isSerialOpened.store(false);
        close(fd_serial);
    }
}

bool PosixSerial::initSerial(std::string& serial_name, SerialParamStruct aStruct) {
    if(isSerialOpened.load()){
        LOG_INFO << "serial already opened...";
        return true;
    }

    if(serial_name.empty())
    {
        LOG_RED << "port name is empty";
        return false;
    }

    std::string realComName = "/dev/" + serial_name;
    fd_serial = open(realComName.c_str(), O_RDWR|O_NOCTTY|O_NDELAY);
    if(fd_serial < 0)
    {
        LOG_RED << realComName << " open error: " << errno;
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

    setSpeed(aStruct.baudrate);
    if(setParity(aStruct.databits, aStruct.stopbits, aStruct.parity) == false)
    {
        LOG_RED << "Set Parity Error";
        close(fd_serial);
        return false;
    }

    //串口成功打开
    isSerialOpened.store(true);

    return true;
}

bool PosixSerial::setParity(int databits, int stopbits, int parity) {
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
            LOG_RED << "setParity Error, <unsupported data size>...";
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
            fprintf(stderr,"Unsupported parity\n");
            return (false);
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
            fprintf(stderr,"Unsupported stop bits\n");
            return (false);
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
        perror("SetupSerial 3");
        return false;
    }

    return true;
}

bool PosixSerial::setSpeed(int speed) const{
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



