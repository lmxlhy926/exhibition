//
// Created by 78472 on 2022/6/15.
//

#ifndef EXHIBITION_DOWNBINARYCMD_H
#define EXHIBITION_DOWNBINARYCMD_H

#include <string>
#include "qlibc/QData.h"
#include "downBinaryUtil.h"

using namespace std;

class DownBinaryCmd{
public:
    /**
     * 将json格式控制命令转换为二进制控制命令
     * @param data      控制命令
     * @param buf       二进制命令数组
     * @param bufSize   数组容量
     * @return          二进制命令长度
     */
    static size_t getBinary(qlibc::QData& data, unsigned char* buf, size_t bufSize);

    //向串口发送二进制命令
    static bool serialSend(unsigned char *buf, int size);

    //将json格式控制命令转换为二进制格式，并向串口发送。
    static bool transAndSendCmd(QData &controlData);
};

class JsonCmd2Binary{
    virtual size_t getBinary(unsigned char* buf, size_t bufSize) = 0;
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


class LightOnOff : public JsonCmd2Binary{
private:
    string pseudoCommand;
    string address;
public:
    explicit LightOnOff(qlibc::QData& data){ init(data); }

    void init(qlibc::QData& data){
        pseudoCommand  = data.getString("command");
        address = data.getString("device_id");
    }

    size_t getBinary(unsigned char* buf, size_t bufSize) override{
        qlibc::QData thisBleConfigData = bleConfig::getInstance()->getBleParamData();
        thisBleConfigData.asValue()["commonBase"]["param"]["ADDRESS_DEST"] = address;
        thisBleConfigData.asValue()["commonBase"]["param"]["OPERATION"] = pseudoCommand;

        string binaryString = DownBinaryUtil::getBinaryString(thisBleConfigData);
        return DownBinaryUtil::binaryString2binary(binaryString, buf, bufSize);
    }
};



#endif //EXHIBITION_DOWNBINARYCMD_H
