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

    int avail(){
        return static_cast<int>(binaryString_.size() - readIndex);
    }

    void reset() {readIndex = 0; }

    void rollBack(size_t nBytes) {
        if(readIndex - nBytes * 2 >= 0)
            readIndex -= nBytes * 2;
    }

    string remainingString(){ return binaryString_.substr(readIndex); }
};

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


class EventTable{
public:
    Event scanResultEvent;                      //扫描结果上报事件
    Event nodeAddressAssignSuccessEvent;        //节点地址分配成功事件
    Event bindSuccessEvent;                     //绑定成功事件
    Event unbindSuccessEvent;                   //成功解绑事件
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
};


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
//            LOG_GREEN << "<<===: scanResult Event, deviceSn = " << deviceSn;
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
        SnAddressMap::getInstance()->deleteDeviceSn(deviceSn);
        LOG_PURPLE << "<<===: unbind device<" << deviceSn <<  "> operation success.....";


        qlibc::QData unbindData;
        unbindData.setString("unicastAddr", unicastAddress);
        unbindData.setString("groupAddr", groupAddress);
        EventTable::getInstance()->unbindSuccessEvent.putData(unbindData);
        EventTable::getInstance()->unbindSuccessEvent.notify_one();
    }

private:
    void init(){
        ReadBinaryString rs(sourceData);
        rs.read2Byte(unicastAddress);
        rs.read2Byte(groupAddress);
    }
};

class LightOnOffStatus{
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
    string construct();
private:
    void init();
};

class LightBrightStatus{
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
    string construct();
private:
    void init();
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
