//
// Created by WJG on 2022-6-1.
//

#include <unistd.h>
#include <termio.h>

#include "PosixSerialPort.h"
#include "log_tool.h"

const static int speed_arr[] = {B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300};
const static int name_arr[] = {115200, 38400, 19200, 9600, 4800, 2400, 1200, 300};

PosixSerialPort::PosixSerialPort():BaseSerialPort(){
    fd_serial = -1;
    is_opened = false;
}

PosixSerialPort::~PosixSerialPort(){
    if(fd_serial > 0)
        close(fd_serial);
}

bool PosixSerialPort::initSerial(std::string serial_name, SerialParamStruct aStruct){

    if(serial_name.empty())
    {
        ipp_LogE("port name is empty");
        return false;
    }
    serial_port_name = serial_name;

    struct termios options{};
    std::string realComName = "/dev/" + serial_name;

    fd_serial = open(realComName.c_str(), O_RDWR|O_NOCTTY|O_NDELAY);
    if(fd_serial < 0)
    {
        ipp_LogE("%s open error:%d",realComName.c_str(), errno);
        return false;
    }
    int res = fcntl(fd_serial, F_SETFL, 0);
    if(res)
    {
        ipp_LogE("fcntl failed\n");
        return false;
    }

    if(isatty(STDIN_FILENO)==0)
    {
        ipp_LogE("input is not a terminal devices\n");
        return false;
    }

    setSpeed(aStruct.baudrate);
    if(setParity(aStruct.databits, aStruct.stopbits, aStruct.parity) == false)
    {
        ipp_LogE("Set Parity Error\n");
        return false;
    }
    is_opened = true;
    return true;
}

bool PosixSerialPort::writeSerialData(uint8_t *buff, int32_t len) {
    if(!is_opened)
        return false;

    int8_t leftTry = ival_comm_write_try_times_;
    int32_t nLeft = len;

    if (buff == nullptr || len <= 0 || fd_serial <= 0) {
        ipp_LogE("data is null or len <=0 or com port not open.");
        return false;
    }

    bool bRet = false;
    uint8_t start[128] = {0x00};
    memcpy(start, buff, len);
    uint8_t * ptr = start;
    while (leftTry--)
    {
        int32_t ret = write(fd_serial, ptr, nLeft);
        if(ret <= 0)
        {
            ipp_LogE("write error!\n");
            bRet = false;
            break;
        }
        nLeft -= ret;
        if(nLeft <= 0)
        {
            bRet = true;
            break;
        }
        ptr += ret;
    }
    if (!bRet)
    {
        ipp_LogE("com is not used or error.");
        return false;
    }
    return true;
}

void PosixSerialPort::readSerialData() {

    if(is_opened && (fd_serial >0))
    {
        m_bThread_alive = true;
        int32_t BytesToRead;
        static struct timespec tm{0, 100000};   //100 ms per try.
        while (m_bThread_alive)
        {
            BytesToRead = ival_comm_buff_size - mBuffLen;
            BytesToRead = read(fd_serial, (mSerialDataBuff+mBuffLen), BytesToRead);
            if (BytesToRead > 0)
            {
                mBuffLen += BytesToRead;
                onSerialDataRead();
            }
            nanosleep(&tm,nullptr);
        }
    }
}

void PosixSerialPort::setSpeed(int speed){
    int   i;
    int   status;
    struct termios   Opt;
    tcgetattr(fd_serial, &Opt);
    for(i = 0; i < sizeof(speed_arr)/sizeof(int); i++)
    {
        if(speed == name_arr[i])
        {
            tcflush(fd_serial, TCIOFLUSH);
            cfsetispeed(&Opt, speed_arr[i]);
            cfsetospeed(&Opt, speed_arr[i]);
            status = tcsetattr(fd_serial, TCSANOW, &Opt);
            if (status != 0)
            {
                perror("tcsetattr fd1");
                return;
            }
            tcflush(fd_serial, TCIOFLUSH);
        }
    }
}

bool PosixSerialPort::setParity(int databits, int stopbits, int parity)
{
    struct termios options;
    if (tcgetattr(fd_serial, &options) != 0)
    {
        perror("SetupSerial 1");
        return(false);
    }
    options.c_cflag &= ~CSIZE;
    switch (databits) /*设置数据位数*/
    {
        case 7:
            options.c_cflag |= CS7;
            break;
        case 8:
            options.c_cflag |= CS8;
            break;
        default:
            fprintf(stderr,"Unsupported data size\n"); return (false);
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
            options.c_cflag &= ~CSTOPB;break;
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
