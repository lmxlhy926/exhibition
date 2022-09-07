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
#include "logic/snAddressMap.h"
#include "siteService/service_site_manager.h"
#include "../parameter.h"
#include "bleConfig.h"
#include "common/httpUtil.h"

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
private:
    static EventTable* eventTable;

    EventTable() = default;
public:
    static EventTable* getInstance(){
        if(eventTable == nullptr){
            eventTable = new EventTable;
            return eventTable;
        }else{
            return eventTable;
        }
    }
};


class ReportEvent{
public:
    virtual void postEvent() = 0;

    static string spaceIntervalFormat(string& str){
        stringstream ss;
        for(int i = 0; i < str.size() / 2; i++){
            ss << str.substr(i * 2, 2);
            if(i < str.size() / 2 - 1)
                ss << " ";
        }
       return ss.str();
    }

    static void publishState(string& device_id, string state_id, Json::Value state_value){
        Json::Value stateItem, state_list, deviceItem, device_list, content;
        stateItem["state_id"] = state_id;
        stateItem["state_value"] = state_value;

        state_list[0] = stateItem;

        deviceItem["device_id"] = device_id;
        deviceItem["online_state"] = "online";
        deviceItem["state_list"] = state_list;

        device_list[0] = deviceItem;

        qlibc::QData publishData;
        publishData.setString("message_id", Device_State_Changed);
        publishData.putData("content", qlibc::QData(device_list));
        ServiceSiteManager::getInstance()->publishMessage(Device_State_Changed, publishData.toJsonString());
        LOG_INFO << "Device_State_Changed: " << publishData.toJsonString();
    }
};


//单个设备扫描结果
class ScanResult : ReportEvent{
private:
    string sourceData;
    string deviceSn;
public:
    explicit ScanResult(string data) : sourceData(std::move(data)){
        init();
    }

    void postEvent() override{
        qlibc::QData data;
        data.setString("deviceSn", deviceSn);
        if(!deviceSn.empty()){
            LOG_YELLOW << "<<===: scanResult Event, deviceSn = " << deviceSn;
            EventTable::getInstance()->scanResultEvent.putData(data);
            EventTable::getInstance()->scanResultEvent.notify_one();
        }
    }
private:
    void init(){
        ReadBinaryString rs(sourceData);
        rs.readBytes(deviceSn, 6);
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
    bool eventAck{false};
public:
    explicit BindResult(string data) : sourceData(std::move(data)){
        init();
    }

    void postEvent() override{
        if(eventAck){
            LOG_GREEN << "<<===: bind operation success.....";
            EventTable::getInstance()->bindSuccessEvent.notify_one();
        }
    }

private:
    void init(){
        ReadBinaryString rs(sourceData);
        string dest;
        rs.readByte(dest);
        if(dest == "00")
            eventAck = true;
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
        LOG_GREEN << "<<===: unbind device<" << deviceSn <<  "> operation success.....";

        //更新config白名单列表
        Json::Value deviceItem, device_list, device_list_content;
        deviceItem["device_id"] = deviceSn;
        device_list[0] = deviceItem;
        device_list_content["device_list"] = device_list;

        qlibc::QData request;
        request.setString("service_id", "whiteListDeleteRequest");
        request.putData("request", qlibc::QData(device_list_content));
        qlibc::QData response;
        SiteRecord::getInstance()->sendRequest2Site(ConfigSiteName, request, response);
        LOG_HLIGHT << "==>deleteDeviceList2ConfigSite: " << request.toJsonString();

        //发布设备解绑消息
        qlibc::QData content, publishData;
        content.setString("device_id", deviceSn);
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
            if(present_onOff == "00"){
                status.setString("state_value", "off");
                state_value = "off";
            }else{
                status.setString("state_value", "on");
                state_value = "on";
            }

            LOG_GREEN << "==>LightOnOffStatus: " << status.toJsonString();

            //更新状态列表
            bleConfig::getInstance()->updateStatusListData(status);

            //发布状态变更消息
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

        Json::Value state_value =  stoi(present_lightness, nullptr, 16);
        qlibc::QData status;
        status.setString("device_id", device_id);
        status.setString("state_id", "luminance");
        status.setValue("state_value", state_value);
        LOG_GREEN << "LightBrightStatus: " << status.toJsonString();

        //更新状态列表
        bleConfig::getInstance()->updateStatusListData(status);
        if(state_value.asInt() > 0){
            status.setString("state_id", "power");
            status.setValue("state_value", "on");
            bleConfig::getInstance()->updateStatusListData(status);

        }else if(state_value.asInt() == 0){
            status.setString("state_id", "power");
            status.setValue("state_value", "off");
            bleConfig::getInstance()->updateStatusListData(status);
        }

        //发布状态变更消息
        publishState(device_id, "luminance", state_value);
        if(state_value.asInt() > 0){
            publishState(device_id, "power", "on");
        }else if(state_value.asInt() == 0){
            publishState(device_id, "power", "off");
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
    string present_lightness;
    string target_lightness;
    string remaining_time;
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
        status.setString("state_id", "luminance");
        status.setInt("state_value", stoi(present_lightness, nullptr, 16));

        LOG_GREEN << "LightColorTemperature: " << status.toJsonString();
        bleConfig::getInstance()->updateStatusListData(status);
    }

private:
    void init(){
        LOG_GREEN << "LightColorTemperature: " << spaceIntervalFormat(sourceData);
        ReadBinaryString rs(sourceData);
        rs.read2Byte(unicast_address);
        rs.read2Byte(group_address);
        rs.read2Byte(opcode);
        rs.read2Byte(present_lightness);
        rs.read2Byte(target_lightness);
        rs.readByte(remaining_time);
    }
};


//解析上报的状态数据包，产生上报事件
class PostStatusEvent{
private:
    string statusString;
public:
    explicit PostStatusEvent(string packageString) : statusString(std::move(packageString)){}

    void operator()();
};



#endif //EXHIBITION_STATUSEVENT_H
