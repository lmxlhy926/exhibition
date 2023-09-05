#include "sendBuffer.h"
#include "qlibc/QData.h"

sendBuffer* sendBuffer::Instance = nullptr;

sendBuffer* sendBuffer::getInstance(){
    if(Instance == nullptr){
        Instance = new sendBuffer();
    }
    return Instance;
}

void sendBuffer::enque(const string& command){
    std::lock_guard<std::mutex> lg(Mutex);
    queue.push(command);
    cv.notify_one();
}

void sendBuffer::sendCommand(){
    string command;
    {
        std::unique_lock<std::mutex> ul;
        if(queue.empty()){
            cv.wait(ul, [this]{return !queue.empty();});
        }
        command = queue.front();
        queue.pop();
    }

    Json::Value value;
    value["commandString"] = command;
    qlibc::QData request, response;
    request.setString("service_id", "");
    request.setValue("request", value);
    
    //todo 发送请求

}



