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

//    if(isatty(STDIN_FILENO)==0)
//    {
//        LOG_RED << "input is not a terminal devices";
//        close(fd_serial);
//        return false;
//    }

//    if(!setProperty(aStruct.databits, aStruct.stopbits, aStruct.parity, aStruct.baudrate))
//    {
//        LOG_RED << "Set Property Error";
//        close(fd_serial);
//        return false;
//    }

    if(!setUartProperty())
    {
        LOG_RED << "setUartProperty Error.....";
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
    return true;
}

bool CommonSerial::write2Serial(const void *buff, int writeLen) {
    if (buff == nullptr || writeLen <= 0 || fd_serial <= 0) {
        LOG_RED << "buff is null or len <=0 or com port not open.";
        return false;
    }

    ssize_t nWrite = write(fd_serial, buff, writeLen);
    if(nWrite != writeLen)
    {
        LOG_RED << "write2Serial Error!......";
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
    return false;
}

bool CommonSerial::setUartProperty() {
    struct termios options{};
    if (tcgetattr(fd_serial, &options) != 0)
    {
        LOG_RED << "tcgetattr Error....";
        return false;
    }

    options.c_iflag  &= ~(ICRNL | IXON | INPCK);            //禁止CR->NL转换、禁止输出流控制起作用、禁止输入奇偶校验
    options.c_iflag  |= IGNPAR;
    options.c_oflag  = 0;                                   //禁止输出处理
    options.c_lflag = 0;
//    options.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);    //非规范模式、禁止回显、禁止信号

    options.c_cflag &= ~CSIZE;          //字符大小屏蔽字
    options.c_cflag |= CS8;             //数据位
    options.c_cflag &= ~CSTOPB;         //一位停止位
    options.c_cflag &= ~PARENB;         //禁用奇偶校验
    options.c_cflag |= (CLOCAL);
    options.c_cflag &= ~CRTSCTS;        //不使用硬件流控制
    options.c_cflag &= ~HUPCL;
    options.c_cflag |= CREAD;

    options.c_cc[VTIME] = 1;
    options.c_cc[VMIN] = 100;           //每次读取100个字节，如果在一秒内没有读取到，则返回

    cfsetispeed(&options, B115200);     //输入波特率
    cfsetospeed(&options, B115200);     //输出波特率

    checkCflag(options);
    LOG_PURPLE << "***************";
    checkIflag(options);
    LOG_RED << "------------------";

    tcflush(fd_serial, TCIOFLUSH);
    int status = tcsetattr(fd_serial, TCSANOW, &options);
    if (status != 0)
    {
        LOG_RED << "tcsetattr Error.....";
        return false;
    }

    struct termios setattribute{};
    if (tcgetattr(fd_serial, &setattribute) != 0)
    {
        LOG_RED << "tcgetattr Error....";
        return false;
    }

    checkCflag(setattribute);
    LOG_PURPLE << "***************";
    checkIflag(setattribute);

    LOG_INFO <<  setattribute.c_cc[VTIME] << " " << setattribute.c_cc[VMIN];

    return true;
}

bool CommonSerial::setUartProperty_try() {
    // Create new termios struc, we call it 'tty' for convention
    struct termios tty;

    // Read in existing settings, and handle any error
    if (tcgetattr(fd_serial, &tty) != 0) {
        // printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
        // LOG(ERROR) << "Error" << errno << "from tcgetattr:" << strerror(errno);
        printf("tcgetattr Error.....\n");
        return false;
    }

    // //设置数据位数 8位数据位
    // tty.c_cflag &= ~CSIZE;
    // tty.c_cflag |= CS8;
    // //校验位 无校验位
    // tty.c_cflag &= ~PARENB;
    // tty.c_iflag &= ~INPCK;
    // //设置停止位  1位停止位
    // tty.c_cflag &= ~CSTOPB;
    // tty.c_cflag |= CLOCAL | CREAD;
    // tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    // tty.c_oflag &= ~OPOST;
    // tty.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

    tty.c_cflag |= (CLOCAL | CREAD); //(本地连接（不改变端口所有者)|接收使能)
    tty.c_cflag &= ~CSIZE; //屏蔽字符大小位
    tty.c_cflag &= ~CRTSCTS; //不使用流控制
    tty.c_cflag |= CS8; // 8个数据位
    tty.c_cflag &= ~CSTOPB; //~1位停止位
    tty.c_iflag |= IGNPAR; //忽略奇偶校验错误
    tty.c_iflag &= ~(ICRNL | IXON); //~(将CR映射到NL|启动出口硬件流控)
    tty.c_oflag = 0; //输出模式标志
    tty.c_lflag = 0; //本地模式标志

    // tty.c_cc[VTIME] = 10; // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
    // tty.c_cc[VMIN] = 0;

    // Set in/out baud rate to be 9600
    cfsetispeed(&tty, B115200);
    cfsetospeed(&tty, B115200);

    // Save tty settings, also checking for error
    if (tcsetattr(fd_serial, TCSANOW, &tty) != 0) {
        // printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
        // LOG(ERROR) << "Error" <<  errno << "from tcsetattr:" << strerror(errno);
        printf("tcsetattr Error.....\n");
    }

    return true;
}

void CommonSerial::checkCflag(struct termios options) {
    if(options.c_cflag & CLOCAL){
        LOG_INFO << "CLOCAL: " << "true";
    }
    if(options.c_cflag & CREAD){
        LOG_INFO << "CREAD: " << "true";
    }
    if(options.c_cflag & CSIZE){
        LOG_INFO << "CSIZE: " << "true";
    }
    if(options.c_cflag & CSTOPB){
        LOG_INFO << "CSTOPB: " << "true";
    }
    if(options.c_cflag & HUPCL){
        LOG_INFO << "HUPCL: " << "true";
    }
    if(options.c_cflag & PARENB){
        LOG_INFO << "PARENB: " << "true";
    }
    if(options.c_cflag & PARODD){
        LOG_INFO << "PARODD: " << "true";
    }
}

void CommonSerial::checkIflag(struct termios options) {
    if(options.c_iflag & BRKINT){
        LOG_HLIGHT << "BRKINT: " << "true";
    }
    if(options.c_iflag & ICRNL){
        LOG_HLIGHT << "ICRNL: " << "true";
    }
    if(options.c_iflag & IGNBRK){
        LOG_HLIGHT << "IGNBRK: " << "true";
    }
    if(options.c_iflag & IGNCR){
        LOG_HLIGHT << "IGNCR: " << "true";
    }
    if(options.c_iflag & IGNPAR){
        LOG_HLIGHT << "IGNPAR: " << "true";
    }
    if(options.c_iflag & INLCR){
        LOG_HLIGHT << "INLCR: " << "true";
    }
    if(options.c_iflag & INPCK){
        LOG_HLIGHT << "INPCK: " << "true";
    }
    if(options.c_iflag & ISTRIP){
        LOG_HLIGHT << "ISTRIP: " << "true";
    }
    if(options.c_iflag & IXANY){
        LOG_HLIGHT << "IXANY: " << "true";
    }
    if(options.c_iflag & IXOFF){
        LOG_HLIGHT << "IXOFF: " << "true";
    }
    if(options.c_iflag & IXON){
        LOG_HLIGHT << "IXON: " << "true";
    }
    if(options.c_iflag & PARMRK){
        LOG_HLIGHT << "PARMRK: " << "true";
    }
}



