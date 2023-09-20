#include "sendBuffer.h"
#include "qlibc/QData.h"
#include "log/Logging.h"
#include "common/httpUtil.h"
#include "../param.h"

sendBuffer* sendBuffer::Instance = nullptr;

void sendBuffer::enque(const Json::Value& command){
    std::lock_guard<std::mutex> lg(Mutex);
    queue.push(command);
    cv.notify_one();
}

void sendBuffer::sendCommand(){
    while(true){
        Json::Value command;
        {
            std::unique_lock<std::mutex> ul(Mutex);
            if(queue.empty()){
                cv.wait(ul, [this]{return !queue.empty();});
            }
            command = queue.front();
            queue.pop();
        }
        qlibc::QData request, response;
        request.setString("service_id", "stripPointControl");
        request.setValue("request", command);
        LOG_GREEN << "request: " << request.toJsonString();
        httpUtil::sitePostRequest("127.0.0.1", SynergySitePort, request, response);
        LOG_BLUE << "response: " << response.toJsonString();
    }
}



