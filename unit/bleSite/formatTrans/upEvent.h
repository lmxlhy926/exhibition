//
// Created by 78472 on 2022/7/4.
//

#ifndef EXHIBITION_UPEVENT_H
#define EXHIBITION_UPEVENT_H

#include <mutex>
#include <condition_variable>
#include <atomic>
#include "qlibc/QData.h"

class Event{
private:
    std::mutex mutex_;
    std::condition_variable cond_;
    qlibc::QData data_;
    std::atomic<bool> flag_{false};

public:
    void notify_one(qlibc::QData& data){
        {
            std::lock_guard<std::mutex> lg(mutex_);
            data_ = data;
            flag_.store(true);
        }
        cond_.notify_one();
    }

    qlibc::QData wait(){
        std::unique_lock<std::mutex> ul(mutex_);
        cond_.wait(ul, [this](){
            return flag_.load();
        });
        flag_.store(false);
        return data_;
    }
};

class UpEvent {
public:
    Event lightStatusEvent;
    Event lightBrightnessEvent;
private:
    static UpEvent* upEvent;
    UpEvent() = default;
public:
    static UpEvent* getInstance(){
        if(upEvent == nullptr){
            upEvent = new UpEvent;
            return upEvent;
        }else{
            return upEvent;
        }
    }
};


#endif //EXHIBITION_UPEVENT_H
