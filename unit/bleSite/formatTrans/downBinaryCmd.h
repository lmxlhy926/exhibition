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
#include "logic/groupAddressMap.h"

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
    static string getBinaryString(qlibc::QData& controlData);
};


class JsonCmd2Binary{
protected:
    //获取二进制格式命令
    virtual string getBinaryString() = 0;

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

//扫描
class LightScan : public JsonCmd2Binary{
public:
    string getBinaryString() override{
        return string("E9FF00");
    }
};

//结束扫描
class LightScanEnd : public JsonCmd2Binary{
public:
    string getBinaryString() override{
        return string("E9FF01");
    }
};

//连接
class LightConnect : public JsonCmd2Binary{
private:
    string deviceSn;
public:
    explicit LightConnect(string& sn) : deviceSn(sn){}

    string getBinaryString() override{
        string binaryString = "E9FF08" + deviceSn;
        return binaryString;
    }
};

//给网关分配地址
class LightGatewayAddressAssign : public JsonCmd2Binary{
public:
    string getBinaryString() override{
        string binaryString = "E9FF091112131415161718191A1B1C1D1E1F20000000112233440100";
        return binaryString;
    }
};

//给节点分配地址
class LightNodeAddressAssign : public JsonCmd2Binary{
private:
    string nodeAddress;
public:
    explicit LightNodeAddressAssign(string& nodeAddr) : nodeAddress(nodeAddr){}

    string getBinaryString() override{
        string binaryString = "E9FF0A1112131415161718191A1B1C1D1E1F2000000011223344" + nodeAddress;
        return binaryString;
    }
};

//设备绑定
class LightBind : public JsonCmd2Binary{
public:
    string getBinaryString() override{
        string binaryString = "E9FF0B00000060964771734FBD76E3B40519D1D94A48";
        return binaryString;
    }
};


//设备解绑
class LightUnBind : public JsonCmd2Binary{
private:
    string deviceSn;
public:
    explicit LightUnBind(string& sn) : deviceSn(sn){}

    string getBinaryString() override{
        string addr = SnAddressMap::getInstance()->deviceSn2Address(deviceSn);
        if(addr.empty()){
            return "";
        }
        string binaryString = "E8FF000000000203" + addr + "8049";
        return binaryString;
    }
};

//设备分组
class LightGroup : public JsonCmd2Binary{
private:
    string deviceSn;
    string groupName;
public:
    explicit LightGroup(string& sn, string& name) : deviceSn(sn), groupName(name){}

    string getBinaryString() override{
        string prefix = deleteWhiteSpace(bleConfig::getInstance()->getBleParamData().getString("commonPrefix"));
        string deviceAddress = SnAddressMap::getInstance()->deviceSn2Address(deviceSn);
        if(deviceAddress.empty()){
            return "";
        }
        string stringCmd;
        stringCmd.append(prefix).append(deviceAddress).append("801B");
        stringCmd.append(deviceAddress);
        stringCmd.append(GroupAddressMap::getInstance()->getGroupAddr(groupName));
        stringCmd.append(deleteWhiteSpace("00 10"));
        return stringCmd;
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

    string getBinaryString() override{
        string prefix = deleteWhiteSpace(bleConfig::getInstance()->getBleParamData().getString("commonPrefix"));
        string stringCmd;
        stringCmd.append(prefix).append(deviceAddress).append("8202");
        if(onOff == "on"){
            stringCmd.append("01");
        }else if(onOff == "off"){
            stringCmd.append("00");
        }
        stringCmd.append(deleteWhiteSpace("00 00 00"));

        return stringCmd;
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
        int tempLuminanceVal = data.getInt("commandPara");
        if(0 <= tempLuminanceVal && tempLuminanceVal <= 0xffff){
            luminanceVal = tempLuminanceVal;
        }else{
            luminanceVal = 0xffff;
        }
    }

    string getBinaryString() override{
        string prefix = deleteWhiteSpace(bleConfig::getInstance()->getBleParamData().getString("commonPrefix"));
        stringstream ss;
        ss << std::hex << std::uppercase << std::setw(4) << luminanceVal;

        string stringCmd;
        stringCmd.append(prefix).append(deviceAddress).append("824C").append(ss.str());
        stringCmd.append(deleteWhiteSpace("00 00 00"));

        return stringCmd;
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
        int tempCtlTemperature = data.getInt("commandPara");
        if(2700 <= tempCtlTemperature && tempCtlTemperature <= 6500){
            ctlTemperature = tempCtlTemperature;
        }else{
            ctlTemperature = 6500;
        }
    }

    string getBinaryString() override{
        string prefix = deleteWhiteSpace(bleConfig::getInstance()->getBleParamData().getString("commonPrefix"));
        stringstream ss;
        ss << std::setfill('0') << std::hex << std::uppercase << std::setw(4) << ctlTemperature;
        string ctlTemperatureStr = ss.str();

        string stringCmd;
        stringCmd.append(prefix).append(deviceAddress).append("8264");
        stringCmd.append(ctlTemperatureStr.substr(2, 2)).append(ctlTemperatureStr.substr(0,2 ));
        stringCmd.append(deleteWhiteSpace("00 00"));
        stringCmd.append(deleteWhiteSpace("00 00 00"));

        return stringCmd;
    }
};


#endif //EXHIBITION_DOWNBINARYCMD_H
