//
// Created by WJG on 2022-5-30.
//

#include "Win32SerialPort.h"
#include "log_tool.h"

Win32SerialPort::Win32SerialPort() : BaseSerialPort() {
    m_hCom = nullptr;
    memset(&m_ov, 0, sizeof(OVERLAPPED));
    m_hThr_shut_event = nullptr;
    m_dwComm_events = EV_RXCHAR;
}

Win32SerialPort::~Win32SerialPort() {
    m_bThread_alive = false;
    CloseHandle(m_hCom);
    m_hCom = nullptr;
    memset(&m_ov, 0, sizeof(OVERLAPPED));
    m_hThr_shut_event = nullptr;
    m_dwComm_events = EV_RXCHAR;
}

bool Win32SerialPort::initSerial(std::string serial_name, SerialParamStruct aStruct){

    if(serial_name.empty())
    {
        ipp_LogE("port name is empty");
        return false;
    }
    serial_port_name = serial_name;

#ifdef x64
    wchar_t  szPort[200];
    memset(szPort, 0, 100);
    setlocale(LC_ALL, "zh_CN.UTF-8");
    mbstowcs_s(&convert_len, szPort, 200 - 1, str_1, 100 - 1);
#else
    char  szPort[100];
    memset(szPort, 0, 100);
    sprintf(szPort, R"(\\.\%s)", serial_name.c_str());
#endif

    if (m_hCom != nullptr)
    {
        CloseHandle(m_hCom);
        m_hCom = nullptr;
    }

    //wsprintf(szPort, L"\\\\.\\%s", TEXT(name_->sPortName.c_str()));
    m_hCom = CreateFile(szPort, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);// Async I / O//(LPCWSTR)
    //createfile error:5 拒绝访问，或者没权限，或者文件被其它占用了
    if (m_hCom == INVALID_HANDLE_VALUE)
    {
        int a = GetLastError();
        ipp_LogE("CreateFile error:%d", a);
        return false;
    }

    GetCommTimeouts(m_hCom, &m_CommTimeouts);
    m_CommTimeouts.ReadIntervalTimeout = MAXDWORD;
    m_CommTimeouts.ReadTotalTimeoutMultiplier = 0;
    m_CommTimeouts.ReadTotalTimeoutConstant = 0;
    m_CommTimeouts.WriteTotalTimeoutMultiplier = 1000;
    m_CommTimeouts.WriteTotalTimeoutConstant = 1000;
    SetCommTimeouts(m_hCom, &m_CommTimeouts);


    if(!SetCommMask(m_hCom, m_dwComm_events))
    {
        ipp_LogE("SetCommMask() failed");
        CloseHandle(m_hCom);
        return false;
    }

    if(!GetCommState(m_hCom, &m_dcb))
    {
        ipp_LogE("GetCommState() failed");
        CloseHandle(m_hCom);
        return false;
    }


    m_dcb.DCBlength = sizeof(DCB);
    m_dcb.BaudRate = aStruct.baudrate;
    m_dcb.ByteSize = aStruct.databits;

    switch(aStruct.parity)
    {
        case 'n':
        case 'N':
            m_dcb.Parity = NOPARITY;
            break;
        case 'o':
        case 'O':
            m_dcb.Parity = ODDPARITY;
            break;
        case 'e':
        case 'E':
            m_dcb.Parity = EVENPARITY;
            break;
        case 'S':
        case 's':
            m_dcb.Parity = SPACEPARITY;
            break;
        default:
            m_dcb.Parity = NOPARITY;
    }

    if(aStruct.stopbits==1)
        m_dcb.StopBits = ONESTOPBIT;
    else if(aStruct.stopbits==2)
        m_dcb.StopBits = TWOSTOPBITS;
    else
        m_dcb.StopBits = ONESTOPBIT;

    if(!SetCommState(m_hCom, &m_dcb))
    {
        ipp_LogE("SetCommState() failed");
        CloseHandle(m_hCom);
        return false;
    }

    if(!SetupComm(m_hCom, ival_comm_buff_max, ival_comm_buff_max))
    {
        ipp_LogE("SetupComm() failed");
        CloseHandle(m_hCom);
        return false;
    }

    PurgeComm(m_hCom, PURGE_RXCLEAR | PURGE_TXCLEAR);

    DWORD dwError;
    COMSTAT cs;
    if (!ClearCommError(m_hCom, &dwError, &cs))
    {
        ipp_LogE("ClearCommError() failed");
        CloseHandle(m_hCom);
        return false;
    }

    // create events
    if (m_ov.hEvent != nullptr)
        ResetEvent(m_ov.hEvent);
    m_ov.hEvent = CreateEvent(nullptr, true, false, nullptr);

    if (m_hThr_shut_event != nullptr)
        ResetEvent(m_hThr_shut_event);
    m_hThr_shut_event = CreateEvent(nullptr, true, false, nullptr);

    m_hEvent_array[0] = m_hThr_shut_event;
    m_hEvent_array[1] = m_ov.hEvent;

    m_bThread_alive = false;
    is_opened = true;

    return true;
}

bool Win32SerialPort::writeSerialData(uint8_t *buff, int32_t len) {
//    ipp_LogD("Win32SerialPort try to write data.\n");
    bool bRet = false;
    bool bResult;
    DWORD dwError = 0;
    COMSTAT comstat;
    int8_t count = 5;
    DWORD dwWrite_byte = 0;
    OVERLAPPED ov;
    uint8_t _mdata[128] = {0x00};

    if(!is_opened)
        return false;

    memset(&ov, 0, sizeof(OVERLAPPED));
    ov.hEvent = CreateEvent(nullptr, true, false, nullptr);

    if (buff == nullptr || len <= 0)
    {
        ipp_LogD("data is null or len <=0");
        return false;
    }

    do
    {
        if (m_hCom == nullptr)
        {
            bRet = false;
            break;
        }
        bResult = ClearCommError(m_hCom, &dwError, &comstat);
        if (!bResult)
        {
            bRet = false;
            break;
        }

        if(static_cast<int>(comstat.cbOutQue + len) <= ival_comm_buff_max)
        {
            bRet = true;
            break;
        }
        Sleep(200);
    } while (count--);

    if (!bRet)
    {
        ipp_LogD("com is not used.");
        return false;
    }
    if (count == 0)
    {
        ipp_LogD("send buff is full.");
        return false;
    }

    memcpy(_mdata, buff, len);
    bResult = WriteFile(m_hCom, _mdata, len, &dwWrite_byte, &ov);
    // deal with any error codes
    if (!bResult)
    {
        DWORD dwError = GetLastError();
        switch (dwError)
        {
            case ERROR_IO_PENDING:
                // continue to GetOverlappedResuts()
                break;

            default:
                ipp_LogE(" Write Serial %s err", serial_port_name.c_str());
                bRet = false;
        }
    }
    else
        ipp_LogD("send data len:%d", len);

    return bRet;
}

void Win32SerialPort::readSerialData() {
    bool bResult;
    DWORD dwError = 0;
    DWORD Event = 0;
    COMSTAT comstat;
    DWORD CommEvent = 0;

    m_bThread_alive = true;

    // Clear comm buffers at startup
    if(m_hCom) // check if the port is opened
        PurgeComm(m_hCom, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);

    while(m_bThread_alive)
    {
        bResult = WaitCommEvent(m_hCom, &Event, &m_ov);
        if (!bResult)
        {
            // If WaitCommEvent() returns FALSE, process the last er or to determin
            // the reason..
            switch(dwError = GetLastError())
            {
                case ERROR_IO_PENDING:
                default:
                    //QLog("WaitCommEvent ) er:%d", dwError);
                    break;
            }
        }
        else
        {
            ClearCommError(m_hCom, &dwError, &comstat);
            if(comstat.cbInQue == 0)
                continue;
        } // end if bResult

        Event = WaitForMultipleObjects(2, m_hEvent_array, false, INFINITE);

        switch(Event) {
            case 0:
                m_bThread_alive = false;
                pthread_exit(nullptr);
                break;
            case 1: // read event
                GetCommMask(m_hCom, &CommEvent);
                if (CommEvent & EV_RXCHAR)
                    // Receive character event from port.
                    procSerialData();
                break;
        } // end switch Event
    }//while(true)
}

void Win32SerialPort::procSerialData() {
    BOOL bRead = false;
    BOOL bResult;
    DWORD dwError = 0;
    DWORD BytesRead = 0;
    COMSTAT com_stat;
    OVERLAPPED ov;

    memset(&ov, 0, sizeof(OVERLAPPED));
    ov.hEvent = CreateEvent(nullptr, true, false, nullptr);

    do
    {
        ClearCommError(m_hCom, &dwError, &com_stat);
        BytesRead = com_stat.cbInQue;
        if (com_stat.cbInQue == 0)
            break;

        BytesRead = std::min(BytesRead, static_cast<DWORD>(ival_comm_buff_size - mBuffLen));
        bResult = ReadFile(m_hCom, (mSerialDataBuff + mBuffLen), BytesRead, &BytesRead, &ov);

        if (!bResult)
        {
            switch (dwError = GetLastError())
            {
                case ERROR_IO_PENDING:
                default:
                {
                    // Another error has occ red.Process this error.
                    //QLog("ReadFile() err:%d",dwError);
                    bRead = false;
                    break;
                }
            }
        }
        else
        {
            if (BytesRead > 0 )
            {
                mBuffLen += BytesRead;
                bRead = true;
            }
        }
        if (!bRead)
            break;

        onSerialDataRead();

    } while (m_bThread_alive); // end forever loop
}
