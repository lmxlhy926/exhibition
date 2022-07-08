//
// Created by 78472 on 2022/6/15.
//

#ifndef EXHIBITION_LIGHTCONTROLCMD_H
#define EXHIBITION_LIGHTCONTROLCMD_H

#include <string>
#include "qlibc/QData.h"
#include "JsonCmd2Binary.h"

using namespace std;

/**
 * 依据下发的请求命令获得二进制控制命令
 * @param data      控制命令
 * @param buf       二进制命令数组
 * @param bufSize   数组容量
 * @return          二进制命令长度
 */
size_t bleJsonCmd2Binaray(qlibc::QData& data, unsigned char* buf, size_t bufSize);


class LightScan : public JsonCmd2Binary{
public:
    size_t getBinary(unsigned char* buf, size_t bufSize) override{
        string binaryString = "E9FF00";
        return binaryString2binary(binaryString, buf, bufSize);
    }
};

class LightScanEnd : public JsonCmd2Binary{
public:
    size_t getBinary(unsigned char* buf, size_t bufSize) override{
        string binaryString = "E9FF01";
        return binaryString2binary(binaryString, buf, bufSize);
    }
};

class LightConnect : public JsonCmd2Binary{
private:
    string deviceSn;
public:
    explicit LightConnect(string& sn) : deviceSn(sn){}

    size_t getBinary(unsigned char* buf, size_t bufSize) override{
        string binaryString = "E9FF08" + deviceSn;
        return binaryString2binary(binaryString, buf, bufSize);
    }
};

class LightGatewayAddressAssign : public JsonCmd2Binary{
public:
    size_t getBinary(unsigned char* buf, size_t bufSize) override{
        string binaryString = "E9FF091112131415161718191A1B1C1D1E1F20000000112233440100";
        return binaryString2binary(binaryString, buf, bufSize);
    }
};

class LightNodeAddressAssign : public JsonCmd2Binary{
private:
    string nodeAddress;
public:
    explicit LightNodeAddressAssign(string& nodeAddr) : nodeAddress(nodeAddr){}

    size_t getBinary(unsigned char* buf, size_t bufSize) override{
        string binaryString = "E9FF0A1112131415161718191A1B1C1D1E1F2000000011223344" + nodeAddress;
        return binaryString2binary(binaryString, buf, bufSize);
    }
};

class LightBind : public JsonCmd2Binary{
public:
    size_t getBinary(unsigned char* buf, size_t bufSize) override{
        string binaryString = "E9FF0B00000060964771734FBD76E3B40519D1D94A48";
        return binaryString2binary(binaryString, buf, bufSize);
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

        string binaryString = getBinaryString(thisBleConfigData);
        return binaryString2binary(binaryString, buf, bufSize);
    }
};



#endif //EXHIBITION_LIGHTCONTROLCMD_H
