//
// Created by 78472 on 2022/6/15.
//

#ifndef EXHIBITION_DOWNBINARYCMD_H
#define EXHIBITION_DOWNBINARYCMD_H

#include <string>
#include <regex>
#include <sstream>
#include "qlibc/QData.h"
#include "downBinaryUtil.h"
#include "logic/snAddressMap.h"

using namespace std;

class DownBinaryCmd{
public:
    //将json格式控制命令转换为二进制格式，并向串口发送。
    static bool transAndSendCmd(QData &controlData);

private:
    /**
     * 将json格式控制命令转换为二进制控制命令
     * @param data      控制命令
     * @param buf       二进制命令数组
     * @param bufSize   数组容量
     * @return          二进制命令长度
     */
    static size_t getBinary(qlibc::QData& controlData, unsigned char* buf, size_t bufSize);
};


class JsonCmd2Binary{
protected:
    //获取二进制格式命令
    virtual size_t getBinary(unsigned char* buf, size_t bufSize) = 0;

    //剔除字符串中间的空格
    static string deleteWhiteSpace(string str){
        string retStr;
        regex sep(" ");
        sregex_token_iterator p(str.cbegin(), str.cend(), sep, -1);
        sregex_token_iterator e;
        for(; p != e; ++p){
            retStr.append(*p);
        }
        return retStr;
    }
};

class LightScan : public JsonCmd2Binary{
public:
    size_t getBinary(unsigned char* buf, size_t bufSize) override{
        string binaryString = "E9FF00";
        return DownBinaryUtil::binaryString2binary(binaryString, buf, bufSize);
    }
};

class LightScanEnd : public JsonCmd2Binary{
public:
    size_t getBinary(unsigned char* buf, size_t bufSize) override{
        string binaryString = "E9FF01";
        return DownBinaryUtil::binaryString2binary(binaryString, buf, bufSize);
    }
};

class LightConnect : public JsonCmd2Binary{
private:
    string deviceSn;
public:
    explicit LightConnect(string& sn) : deviceSn(sn){}

    size_t getBinary(unsigned char* buf, size_t bufSize) override{
        string binaryString = "E9FF08" + deviceSn;
        return DownBinaryUtil::binaryString2binary(binaryString, buf, bufSize);
    }
};

class LightGatewayAddressAssign : public JsonCmd2Binary{
public:
    size_t getBinary(unsigned char* buf, size_t bufSize) override{
        string binaryString = "E9FF091112131415161718191A1B1C1D1E1F20000000112233440100";
        return DownBinaryUtil::binaryString2binary(binaryString, buf, bufSize);
    }
};

class LightNodeAddressAssign : public JsonCmd2Binary{
private:
    string nodeAddress;
public:
    explicit LightNodeAddressAssign(string& nodeAddr) : nodeAddress(nodeAddr){}

    size_t getBinary(unsigned char* buf, size_t bufSize) override{
        string binaryString = "E9FF0A1112131415161718191A1B1C1D1E1F2000000011223344" + nodeAddress;
        return DownBinaryUtil::binaryString2binary(binaryString, buf, bufSize);
    }
};

class LightBind : public JsonCmd2Binary{
public:
    size_t getBinary(unsigned char* buf, size_t bufSize) override{
        string binaryString = "E9FF0B00000060964771734FBD76E3B40519D1D94A48";
        return DownBinaryUtil::binaryString2binary(binaryString, buf, bufSize);
    }
};

class LightUnBind : public JsonCmd2Binary{
private:
    string deviceSn;
public:
    explicit LightUnBind(string& sn) : deviceSn(sn){}

    size_t getBinary(unsigned char* buf, size_t bufSize) override{
        string addr = SnAddressMap::getInstance()->deviceSn2Address(deviceSn);
        string binaryString = "E8FF000000000203" + addr + "8049";
        return DownBinaryUtil::binaryString2binary(binaryString, buf, bufSize);
    }
};

//开关
class LightOnOff : public JsonCmd2Binary{
private:
    string deviceAddress;
    string onOff;
public:
    explicit LightOnOff(qlibc::QData& data) { init(data); }

    void init(qlibc::QData& data){
        deviceAddress = data.getString("deviceAddress");
        onOff = data.getString("commandPara");
    }

    size_t getBinary(unsigned char* buf, size_t bufSize) override{
        string prefix = deleteWhiteSpace(bleConfig::getInstance()->getBleParamData().getString("commonPrefix"));
        string stringCmd;
        stringCmd.append(prefix).append(deviceAddress).append("8202");
        if(onOff == "on"){
            stringCmd.append("01");
        }else if(onOff == "off"){
            stringCmd.append("00");
        }
        stringCmd.append(deleteWhiteSpace("00 00 00"));

        return DownBinaryUtil::binaryString2binary(stringCmd, buf, bufSize);
    }
};

//亮度
class LightLuminance : public JsonCmd2Binary{
private:
    string deviceAddress;
    int luminanceVal = 0;

public:
    explicit LightLuminance(qlibc::QData& data){ init(data); }

    void init(qlibc::QData& data){
        deviceAddress = data.getString("deviceAddress");
        luminanceVal = data.getInt("commandPara");
    }

    size_t getBinary(unsigned char* buf, size_t bufSize) override{
        string prefix = deleteWhiteSpace(bleConfig::getInstance()->getBleParamData().getString("commonPrefix"));
        stringstream ss;
        ss << std::hex << std::uppercase << std::setw(4) << luminanceVal;

        string stringCmd;
        stringCmd.append(prefix).append(deviceAddress).append("824C").append(ss.str());
        stringCmd.append(deleteWhiteSpace("00 00 00"));

        return DownBinaryUtil::binaryString2binary(stringCmd, buf, bufSize);
    }
};

//色温
class LightColorTem : public JsonCmd2Binary{
private:
    string deviceAddress;
    int ctlTemperature;

public:
    explicit LightColorTem(qlibc::QData& data){ init(data); }

    void init(qlibc::QData& data){
        deviceAddress = data.getString("deviceAddress");
        ctlTemperature = data.getInt("commandPara");
    }

    size_t getBinary(unsigned char* buf, size_t bufSize) override{
        string prefix = deleteWhiteSpace(bleConfig::getInstance()->getBleParamData().getString("commonPrefix"));
        stringstream ss;
        ss << std::hex << std::uppercase << std::setw(4) << ctlTemperature;

        string stringCmd;
        stringCmd.append(prefix).append(deviceAddress).append("8264").append(ss.str());
        stringCmd.append(deleteWhiteSpace("00 00"));
        stringCmd.append(deleteWhiteSpace("00 00 00"));

        return DownBinaryUtil::binaryString2binary(stringCmd, buf, bufSize);
    }
};


#endif //EXHIBITION_DOWNBINARYCMD_H
