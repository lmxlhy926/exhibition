#include "sendBuffer.h"
#include "qlibc/QData.h"
#include "log/Logging.h"

sendBuffer* sendBuffer::Instance = nullptr;

void sendBuffer::enque(const string& command){
    std::lock_guard<std::mutex> lg(Mutex);
    queue.push(command);
    cv.notify_one();
}

void sendBuffer::sendCommand(){
    while(true){
        string command;
        {
            std::unique_lock<std::mutex> ul(Mutex);
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

        LOG_GREEN << "request: " << request.toJsonString();
        
        //todo 发送请求

    }
}



