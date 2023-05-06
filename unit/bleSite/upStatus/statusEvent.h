//
// Created by 78472 on 2022/7/4.
//

#ifndef EXHIBITION_STATUSEVENT_H
#define EXHIBITION_STATUSEVENT_H

#include <string>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <iostream>
#include "qlibc/QData.h"
#include "log/Logging.h"
#include "sourceManage/snAddressMap.h"
#include "sourceManage/groupAddressMap.h"
#include "siteService/service_site_manager.h"
#include "../parameter.h"
#include "sourceManage/bleConfig.h"
#include "common/httpUtil.h"
#include "deviceTypeExtract.h"
#include "../sourceManage/util.h"

using namespace servicesite;
using namespace std;

class ReadBinaryString{
private:
    string binaryString_;
    size_t readIndex = 0;
public:
    explicit ReadBinaryString(string binaryString)
            : binaryString_(std::move(binaryString)){}

    ReadBinaryString& readByte(string& dest);
    ReadBinaryString& readByte();
    ReadBinaryString& read2Byte(string& dest);
    ReadBinaryString& read2Byte();
    ReadBinaryString& readBytes(string& dest, int readBytesNum);
    ReadBinaryString& readBytes(int readBytesNum);

    int avail();

    void reset() {readIndex = 0; }

    void rollBack(size_t nBytes) {
        if(readIndex - nBytes * 2 >= 0)
            readIndex -= nBytes * 2;
    }

    string remainingString(){ return binaryString_.substr(readIndex); }
};


/*
 * 定义一个事件：
 *      1. 存储数据
 *      2. 通知
 *      3. 等待
 */
class Event{
private:
    std::mutex mutex_;
    std::condition_variable cond_;
    qlibc::QData data_;

public:
    void putData(const qlibc::QData& data){
        std::lock_guard<std::mutex> lg(mutex_);
        data_ = data;
    }

    qlibc::QData getData(){
        std::lock_guard<std::mutex> lg(mutex_);
        return data_;
    }

    void notify_one(){
        cond_.notify_one();
    }

    void notify_all(){
        cond_.notify_all();
    };

    void wait(){
        std::unique_lock<std::mutex> ul(mutex_);
        cond_.wait(ul);
    }

    std::cv_status wait(int64_t seconds){
        std::unique_lock<std::mutex> ul(mutex_);
        return cond_.wait_for(ul, std::chrono::seconds(seconds));
    }
};

//事件表：记录关心的事件
class EventTable{
public:
    Event scanResultEvent;                      //单个扫描结果上报事件
    Event nodeAddressAssignSuccessEvent;        //节点地址分配成功事件
    Event bindSuccessEvent;                     //单个设备绑定成功事件
    Event unbindSuccessEvent;                   //单个设备成功解绑事件
    Event gateWayIndexEvent;                    //拟配置节点响应报告
    Event gateWayNetInfoEvent;                  //网络配置信息报告
private:
    static EventTable* eventTable;

    EventTable() = default;
public:
    static EventTable* getInstance(){
        if(eventTable == nullptr){
            eventTable = new EventTable;
        }
        return eventTable;
    }
};


class ReportEvent{
public:
    virtual void postEvent() = 0;

    //用空格分隔字符串
    static string spaceIntervalFormat(string& str){
        stringstream ss;
        for(int i = 0; i < str.size() / 2; i++){
            ss << str.substr(i * 2, 2);
            if(i < str.size() / 2 - 1)
                ss << " ";
        }
       return ss.str();
    }

    //发布状态消息
    static void publishState(string& device_id, string state_id, Json::Value state_value){
        Json::Value contentItem;
        contentItem["device_id"] = device_id;
        contentItem["state_id"] = state_id;
        contentItem["state_value"] = state_value;
        contentItem["sourceSite"] = util::getSourceSite();

        qlibc::QData publishData;
        publishData.setString("message_id", Device_State_Changed);
        publishData.setValue("content", contentItem);
        ServiceSiteManager::getInstance()->publishMessage(Device_State_Changed, publishData.toJsonString());
        LOG_INFO << "Device_State_Changed: " << publishData.toJsonString();
    }
};


//单个设备扫描结果
class ScanResult : ReportEvent{
private:
    string sourceData;
    string deviceSn;
    string deviceUUID;
public:
    explicit ScanResult(string data) : sourceData(std::move(data)){
        init();
    }

    void postEvent() override{
        deviceTypeExtract extract(deviceUUID);
        qlibc::QData data;
        data.setString("deviceSn", deviceSn);
        data.setString("device_type", extract.getDeviceType());
        data.setString("device_typeCode", extract.getDeviceTypeCode());
        data.setString("device_model", extract.getDeviceModel());
        data.setString("device_modelCode", extract.getDeviceModelCode());

        if(!deviceSn.empty()){
            LOG_GREEN<< "<<===: scanResult Event, deviceSn = " << deviceSn;
            EventTable::getInstance()->scanResultEvent.putData(data);
            EventTable::getInstance()->scanResultEvent.notify_all();
        }
    }
private:
    void init(){
        ReadBinaryString rs(sourceData);
        rs.readBytes(deviceSn, 6);
        rs.readBytes(3).readBytes(deviceUUID, 16);
    }
};

//网关配置响应
class GateWayIndexAck : public ReportEvent{
public:
    void postEvent() override{
        LOG_GREEN << "<<==: gateWay assign operation completed.....";
        EventTable::getInstance()->gateWayIndexEvent.notify_one();
    }
};


//网关网络信息响应
class GateWayNetInfoAck : public ReportEvent{
private:
    string sourceData;
    string status;
    string netKey;

public:
    explicit GateWayNetInfoAck(string data) : sourceData(std::move(data)){
        init();
    }

    void postEvent() override{
        LOG_GREEN << "<<==: GateWayNetInfoAck.....";
        if(status == "01"){
            qlibc::QData data;
            data.setString("netKey", netKey);
            EventTable::getInstance()->gateWayNetInfoEvent.putData(data);
            EventTable::getInstance()->gateWayNetInfoEvent.notify_one();
        }else{
            qlibc::QData data;
            EventTable::getInstance()->gateWayNetInfoEvent.putData(data);
            EventTable::getInstance()->gateWayNetInfoEvent.notify_one();
        }
    }

private:
    void init(){
        ReadBinaryString rs(sourceData);
        rs.readByte(status);
        rs.readBytes(netKey, 16);
    }
};


//节点分配成功事件
class NodeAddressAssignAck : public ReportEvent{
private:
    string sourceData;
    bool eventAck{false};
public:
    explicit NodeAddressAssignAck(string data) : sourceData(std::move(data)){
        init();
    }

    void postEvent() override{
        if(eventAck){
            LOG_GREEN << "<<==: nodeAddress assign operation completed.....";
            EventTable::getInstance()->nodeAddressAssignSuccessEvent.notify_one();
        }
    }

private:
    void init(){
        ReadBinaryString rs(sourceData);
        string dest;
        rs.readByte(dest);
        if(dest == "08")
            eventAck = true;
    }
};


//绑定结果事件
class BindResult : public ReportEvent{
private:
    string sourceData;
    string dest;
public:
    explicit BindResult(string data) : sourceData(std::move(data)){
        init();
    }

    void postEvent() override{
        if(dest == "00"){
            LOG_GREEN << "<<===: bind operation success.....";
            qlibc::QData data;
            data.setBool("bind", true);
            EventTable::getInstance()->bindSuccessEvent.putData(data);
            EventTable::getInstance()->bindSuccessEvent.notify_one();

        }else if(dest == "01"){
            LOG_RED << "<<===: bind operation failed.....";
            qlibc::QData data;
            data.setBool("bind", false);
            EventTable::getInstance()->bindSuccessEvent.putData(data);
            EventTable::getInstance()->bindSuccessEvent.notify_one();
        }
    }

private:
    void init(){
        ReadBinaryString rs(sourceData);
        rs.readByte(dest);
    }
};


//单个设备解绑事件
class UnBindResult : public ReportEvent{
private:
    string sourceData;
    string unicastAddress;
    string groupAddress;
public:
    explicit UnBindResult(string data) : sourceData(std::move(data)){
        init();
    }

    void postEvent() override{
        string deviceSn = SnAddressMap::getInstance()->address2DeviceSn(unicastAddress);
        //从地址列表删除该设备
        SnAddressMap::getInstance()->deleteDeviceSn(deviceSn);
        //从设备列表删除该设备
        bleConfig::getInstance()->deleteDeviceItem(deviceSn);
        //从状态列表移除该设备
        bleConfig::getInstance()->deleteStatusItem(deviceSn);
        //从组列表中删除设备
        GroupAddressMap::getInstance()->removeDeviceFromAnyGroup(deviceSn);
        LOG_YELLOW << "<<===: unbind device<" << deviceSn <<  "> operation success.....";

        //通知设备管理站点进行设备列表更新
        util::updateDeviceList();

        //发布设备解绑消息
        qlibc::QData content, publishData;
        content.setString("device_id", deviceSn);
        content.setString("sourceSite", util::getSourceSite());
        publishData.setString("message_id", SingleDeviceUnbindSuccessMsg);
        publishData.putData("content", content);
        ServiceSiteManager::getInstance()->publishMessage(SingleDeviceUnbindSuccessMsg, publishData.toJsonString());
    }

private:
    void init(){
        ReadBinaryString rs(sourceData);
        rs.read2Byte(unicastAddress);
        rs.read2Byte(groupAddress);
    }
};

//开关状态
class LightOnOffStatus : ReportEvent{
private:
    string sourceData;
    string unicast_address;
    string group_address;
    string opcode;
    string present_onOff;
    string target_onOff;
    string remaining_time;
public:
    explicit LightOnOffStatus(string data) : sourceData(std::move(data)){
        init();
    }

    void postEvent() override{
            string device_id = SnAddressMap::getInstance()->address2DeviceSn(unicast_address);
            if(device_id.empty()){
                return;
            }

            Json::Value state_value;
            qlibc::QData status;
            status.setString("device_id", device_id);
            status.setString("state_id", "power");
            if(target_onOff.empty()){
                if(present_onOff == "00"){
                    status.setString("state_value", "off");
                    state_value = "off";
                }else{
                    status.setString("state_value", "on");
                    state_value = "on";
                }
            }else{
                if(target_onOff == "00"){
                    status.setString("state_value", "off");
                    state_value = "off";
                }else{
                    status.setString("state_value", "on");
                    state_value = "on";
                }
            }

            LOG_GREEN << "==>LightOnOffStatus: " << status.toJsonString();
            //更新状态列表，发布状态变更消息
            bleConfig::getInstance()->updateStatusListData(status);
            publishState(device_id, "power", state_value);
    }

private:
    void init(){
        LOG_GREEN << "==>LightOnOffStatus: " << spaceIntervalFormat(sourceData);
        ReadBinaryString rs(sourceData);
        rs.read2Byte(unicast_address);
        rs.read2Byte(group_address);
        rs.read2Byte(opcode);
        rs.readByte(present_onOff);
        rs.readByte(target_onOff);
        rs.readByte(remaining_time);
    }
};


//亮度状态
class LightBrightStatus : ReportEvent{
private:
    string sourceData;
    string unicast_address;
    string group_address;
    string opcode;
    string present_lightness;
    string target_lightness;
    string remaining_time;
public:
    explicit LightBrightStatus(string data) : sourceData(std::move(data)){
        init();
    }

    void postEvent() override{
        string device_id = SnAddressMap::getInstance()->address2DeviceSn(unicast_address);
        if(device_id.empty()){
            return;
        }

        string realLightness;
        realLightness.append(present_lightness.substr(2, 2)).append(present_lightness.substr(0, 2));
        int lightnessInt;
        try{
            lightnessInt = stoi(realLightness, nullptr, 16);
        }catch(const exception& e){
            lightnessInt = 0;
        }
        Json::Value state_value =  lightnessInt;
        qlibc::QData status;
        status.setString("device_id", device_id);
        status.setString("state_id", "luminance");
        status.setValue("state_value", state_value);
        LOG_GREEN << "LightBrightStatus: " << status.toJsonString();
        bleConfig::getInstance()->updateStatusListData(status);
        publishState(device_id, "luminance", state_value);

        if(state_value.asInt() > 0){
            status.setString("state_id", "power");
            status.setValue("state_value", "on");
            bleConfig::getInstance()->updateStatusListData(status);

        }else if(state_value.asInt() == 0){
            status.setString("state_id", "power");
            status.setValue("state_value", "off");
            bleConfig::getInstance()->updateStatusListData(status);
        }
    }

private:
    void init(){
        LOG_GREEN << "LightBrightStatus: " << spaceIntervalFormat(sourceData);
        ReadBinaryString rs(sourceData);
        rs.read2Byte(unicast_address);
        rs.read2Byte(group_address);
        rs.read2Byte(opcode);
        rs.read2Byte(present_lightness);
        rs.read2Byte(target_lightness);
        rs.readByte(remaining_time);
    }
};


//色温状态
class LightColorTemperature : ReportEvent{
private:
    string sourceData;
    string unicast_address;
    string group_address;
    string opcode;
    string present_temperature;
    string transTemperature;
public:
    explicit LightColorTemperature(string data) : sourceData(std::move(data)){
        init();
    }

    void postEvent() override{
        string device_id = SnAddressMap::getInstance()->address2DeviceSn(unicast_address);
        if(device_id.empty()){
            return;
        }

        qlibc::QData status;
        status.setString("device_id", device_id);
        status.setString("state_id", "color_temperature");
        transTemperature.append(present_temperature.substr(2, 2)).append(present_temperature.substr(0, 2));
        int temperatureInt;
        try{
            temperatureInt = stoi(transTemperature, nullptr, 16);
        }catch(const exception& e){
            temperatureInt = 0;
        }
        status.setInt("state_value", temperatureInt);

        LOG_GREEN << "LightColorTemperature: " << status.toJsonString();
        bleConfig::getInstance()->updateStatusListData(status);
        publishState(device_id, "color_temperature", temperatureInt);
    }

private:
    void init(){
        LOG_GREEN << "LightColorTemperature: " << spaceIntervalFormat(sourceData);
        ReadBinaryString rs(sourceData);
        rs.read2Byte(unicast_address);
        rs.read2Byte(group_address);
        rs.read2Byte(opcode);
        rs.read2Byte(present_temperature);
    }
};

//亮度、色温联合控制上报
class LightBrightColorTemperature : ReportEvent{
private:
    string sourceData;
    string unicast_address;
    string group_address;
    string opcode;
    string lightness;
    string color_temperatrue;
public:
    explicit LightBrightColorTemperature(string data) : sourceData(std::move(data)){
        init();
    }

    void postEvent() override{
        string device_id = SnAddressMap::getInstance()->address2DeviceSn(unicast_address);
        if(device_id.empty()){
            return;
        }

        //处理亮度信息
        qlibc::QData brightnessStatus;
        string realLightness;
        realLightness.append(lightness.substr(2, 2)).append(lightness.substr(0, 2));
        int lightnessInt;
        try{
            lightnessInt = stoi(realLightness, nullptr, 16);
        }catch(const exception& e){
            lightnessInt = 0;
        }
        Json::Value state_value = lightnessInt;
        brightnessStatus.setString("device_id", device_id);
        brightnessStatus.setString("state_id", "luminance");
        brightnessStatus.setValue("state_value", state_value);
        LOG_GREEN << "LightBrightColorTemperature: " << brightnessStatus.toJsonString();
        bleConfig::getInstance()->updateStatusListData(brightnessStatus);
        publishState(device_id, "luminance", state_value);

        if(state_value.asInt() > 0){
            brightnessStatus.setString("state_id", "power");
            brightnessStatus.setValue("state_value", "on");
            bleConfig::getInstance()->updateStatusListData(brightnessStatus);

        }else if(state_value.asInt() == 0){
            brightnessStatus.setString("state_id", "power");
            brightnessStatus.setValue("state_value", "off");
            bleConfig::getInstance()->updateStatusListData(brightnessStatus);
        }

        //处理色温消息
        qlibc::QData  colorTemperatureStatus;
        string realColorTemperature;
        realColorTemperature.append(color_temperatrue.substr(2, 2)).append(color_temperatrue.substr(0, 2));
        int colorTemperatureInt;
        try{
            colorTemperatureInt = stoi(realColorTemperature, nullptr, 16);
        }catch(const exception& e){
            colorTemperatureInt = 0;
        }

        colorTemperatureStatus.setString("device_id", device_id);
        colorTemperatureStatus.setString("state_id", "color_temperature");
        colorTemperatureStatus.setInt("state_value", colorTemperatureInt);
        LOG_GREEN << "LightBrightColorTemperature: " << colorTemperatureStatus.toJsonString();
        bleConfig::getInstance()->updateStatusListData(colorTemperatureStatus);
        publishState(device_id, "color_temperature", colorTemperatureInt);
    }

private:
    void init(){
        LOG_GREEN << "LightBrightColorTemperature: " << spaceIntervalFormat(sourceData);
        ReadBinaryString rs(sourceData);
        rs.read2Byte(unicast_address);
        rs.read2Byte(group_address);
        rs.read2Byte(opcode);
        rs.read2Byte(lightness);
        rs.read2Byte(color_temperatrue);
    }
};

#endif //EXHIBITION_STATUSEVENT_H
