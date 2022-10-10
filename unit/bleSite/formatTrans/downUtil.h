//
// Created by 78472 on 2022/6/15.
//

#ifndef EXHIBITION_DOWNUTIL_H
#define EXHIBITION_DOWNUTIL_H

#include <string>
#include <regex>
#include <sstream>
#include <iomanip>
#include "qlibc/QData.h"
#include "logic/snAddressMap.h"
#include "logic/groupAddressMap.h"
#include "../parameter.h"
#include "formatTrans/bleConfig.h"

using namespace std;

class DownUtility{
public:
    //将json格式控制命令转换为二进制字符串，并发送
    static bool parse2Send(qlibc::QData &controlData);

private:
    //将json格式控制命令转换为二进制字符串
    static string cmdData2BinaryCommandString(qlibc::QData& controlData);
};


class BuildBinaryString{
private:
    static string commandPrefix;
protected:
    //获取二进制格式命令
    virtual string getBinaryString() = 0;
public:
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

    string getCommandPrefix(){
        return deleteWhiteSpace(commandPrefix);
    }
};

//扫描
class LightScan : public BuildBinaryString{
public:
    string getBinaryString() override{
        return string("E9FF00");
    }
};

//结束扫描
class LightScanEnd : public BuildBinaryString{
public:
    string getBinaryString() override{
        return string("E9FF01");
    }
};

//连接
class LightConnect : public BuildBinaryString{
private:
    string deviceSn;
public:
    explicit LightConnect(string& sn) : deviceSn(sn){}

    string getBinaryString() override{
        string binaryString = "E9FF08" + deviceSn;
        return binaryString;
    }
};

//给网关分配地址; 《b10展厅11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F 2A》
class LightGatewayAddressAssign : public BuildBinaryString{
public:
    string getBinaryString() override{
        string network_key = "11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F 2A";
        string binaryString = "E9 FF 09" + network_key + "00 00" + "00" + "11 22 33 44" + "00 01";
        return deleteWhiteSpace(binaryString);
    }
};

//给节点分配地址; 《b10展厅11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F 2A》
class LightNodeAddressAssign : public BuildBinaryString{
private:
    string nodeAddress;
public:
    explicit LightNodeAddressAssign(string& nodeAddr) : nodeAddress(nodeAddr){}

    string getBinaryString() override{
        string network_key = "11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F 2A";
        string binaryString = "E9 FF 0A" + network_key + "00 00" + "00" + "11 22 33 44" + nodeAddress;
        return deleteWhiteSpace(binaryString);
    }
};

//设备绑定
class LightBind : public BuildBinaryString{
public:
    string getBinaryString() override{
        string binaryString = "E9FF0B00000060964771734FBD76E3B40519D1D94A48";
        return binaryString;
    }
};


//设备解绑
class LightUnBind : public BuildBinaryString{
private:
    string deviceSn;
public:
    explicit LightUnBind(string& sn) : deviceSn(sn){}

    string getBinaryString() override{
        string deviceAddress = SnAddressMap::getInstance()->deviceSn2Address(deviceSn);
        if(deviceAddress.empty()){
            return "";
        }
        string binaryString = "E8FF000000000203" + deviceAddress + "8049";
        return binaryString;
    }
};

//设备分组
class LightAdd2Group : public BuildBinaryString{
private:
    string deviceSn;
    string group_id;
    string model_name;
public:
    explicit LightAdd2Group(string& sn, string& address, string& modelName)
                            : deviceSn(sn), group_id(address), model_name(modelName){}

    string getBinaryString() override{
        string prefix = getCommandPrefix();
        string deviceAddress = SnAddressMap::getInstance()->deviceSn2Address(deviceSn);
        if(deviceAddress.empty() || group_id.empty()){
            return "";
        }

        //添加设备进入分组
        GroupAddressMap::getInstance()->addDevice2Group(group_id, deviceSn);
        //分组信息加入设备列表
        string group_name = GroupAddressMap::getInstance()->groupAddressId2GroupName(group_id);
        bleConfig::getInstance()->insertGroupInfo(deviceSn, group_name, group_id);

        string stringCmd;
        stringCmd.append(prefix).append(deviceAddress).append("801B");
        stringCmd.append(deviceAddress);
        stringCmd.append(group_id);
        if(model_name == POWER){
            stringCmd.append(deleteWhiteSpace("00 10"));

        }else if(model_name == LUMINANCE){
            stringCmd.append(deleteWhiteSpace("00 13"));

        }else if(model_name == COLORTEMPERATURE){
            stringCmd.append(deleteWhiteSpace("06 13"));
        }

        return stringCmd;
    }
};


//设备解除分组
class LightDelFromGroup : public BuildBinaryString{
private:
    string deviceSn;
    string group_id;
    string model_name;
public:
    explicit LightDelFromGroup(string& sn, string& address, string& modelName)
                            : deviceSn(sn), group_id(address), model_name(modelName){}

    string getBinaryString() override{
        string prefix = getCommandPrefix();
        string deviceAddress = SnAddressMap::getInstance()->deviceSn2Address(deviceSn);
        if(deviceAddress.empty() || group_id.empty()){
            return "";
        }

        //设备从分组剔除
        GroupAddressMap::getInstance()->removeDeviceFromGroup(group_id, deviceSn);
        //分组信息从设备列表中移除
        bleConfig::getInstance()->deleteGroupInfo(deviceSn);

        string stringCmd;
        stringCmd.append(prefix).append(deviceAddress).append("801C");
        stringCmd.append(deviceAddress);
        stringCmd.append(group_id);

        if(model_name == POWER){
            stringCmd.append(deleteWhiteSpace("00 10"));

        }else if(model_name == LUMINANCE){
            stringCmd.append(deleteWhiteSpace("00 13"));

        }else if(model_name == COLORTEMPERATURE){
            stringCmd.append(deleteWhiteSpace("06 13"));
        }
        return stringCmd;
    }
};


//开关
class LightOnOff : public BuildBinaryString{
private:
    string address;
    string onOff;
    string transTime;
public:
    explicit LightOnOff(qlibc::QData& data) { init(data); }

    void init(qlibc::QData& data){
        address = data.getString("address");
        onOff = data.getString("commandPara");

        int transTimeInt = data.getInt("transTime");
        if(0 <= transTimeInt && transTimeInt <= 62){
            stringstream ss;
            ss << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << transTimeInt;
            transTime = ss.str();
        }else{
            transTime = "3E";
        }
    }

    string getBinaryString() override{
        string prefix = getCommandPrefix();
        string stringCmd;
        stringCmd.append(prefix).append(address).append("8202");
        if(onOff == "on"){
            stringCmd.append("01");
        }else if(onOff == "off"){
            stringCmd.append("00");
        }
        stringCmd.append("00");
        stringCmd.append(transTime);
        stringCmd.append("00");
        return stringCmd;
    }
};


//亮度
class LightLuminance : public BuildBinaryString{
private:
    string address;
    int luminanceVal = 0;
    string transTime;

public:
    explicit LightLuminance(qlibc::QData& data){ init(data); }

    void init(qlibc::QData& data){
        address = data.getString("address");
        int tempLuminanceVal = data.getInt("commandPara");
        if(0 <= tempLuminanceVal && tempLuminanceVal <= 0xffff){
            luminanceVal = tempLuminanceVal;
        }else{
            luminanceVal = 0xffff;
        }

        int transTimeInt = data.getInt("transTime");
        if(0 <= transTimeInt && transTimeInt <= 62){
            stringstream ss;
            ss << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << transTimeInt;
            transTime = ss.str();
        }else{
            transTime = "3E";
        }
    }

    string getBinaryString() override{
        string prefix = getCommandPrefix();
        stringstream ss;
        ss << std::hex << std::uppercase << std::setw(4) << luminanceVal;

        string stringCmd;
        stringCmd.append(prefix).append(address).append("824C").append(ss.str());
        stringCmd.append("00");
        stringCmd.append(transTime);
        stringCmd.append("00");

        return stringCmd;
    }
};


//色温
class LightColorTem : public BuildBinaryString{
private:
    string address;
    int ctlTemperature;
    string transTime;

public:
    explicit LightColorTem(qlibc::QData& data){ init(data); }

    void init(qlibc::QData& data){
        address = data.getString("address");
        int tempCtlTemperature = data.getInt("commandPara");
        if(2700 <= tempCtlTemperature && tempCtlTemperature <= 6500){
            ctlTemperature = tempCtlTemperature;
        }else{
            ctlTemperature = 6500;
        }

        int transTimeInt = data.getInt("transTime");
        if(0 <= transTimeInt && transTimeInt <= 62){
            stringstream ss;
            ss << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << transTimeInt;
            transTime = ss.str();
        }else{
            transTime = "3E";
        }
    }

    string getBinaryString() override{
        string prefix = getCommandPrefix();
        stringstream ss;
        ss << std::setfill('0') << std::hex << std::uppercase << std::setw(4) << ctlTemperature;
        string ctlTemperatureStr = ss.str();

        string stringCmd;
        stringCmd.append(prefix).append(address).append("8264");
        stringCmd.append(ctlTemperatureStr.substr(2, 2)).append(ctlTemperatureStr.substr(0,2 ));
        stringCmd.append(deleteWhiteSpace("00 00"));
        stringCmd.append("00");
        stringCmd.append(transTime);
        stringCmd.append("00");

        return stringCmd;
    }
};


#endif //EXHIBITION_DOWNUTIL_H
