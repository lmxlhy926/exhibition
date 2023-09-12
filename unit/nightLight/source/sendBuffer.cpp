#include "sendBuffer.h"
#include "qlibc/QData.h"
#include "log/Logging.h"
#include "common/httpUtil.h"

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
        qlibc::QData request, response;
        Json::Value value;
        value["commandString"] = command;
        request.setString("service_id", "send2CmdBuffer");
        request.setValue("request", value);
        LOG_GREEN << "request: " << request.toJsonString();
        httpUtil::sitePostRequest("127.0.0.1", 9006, request, response);
        LOG_YELLOW << "response: " << response.toJsonString();

        //todo 
        /**
         * 命令最终要发送给设备管理站点
         * 但是设备是在蓝牙站点上的
         * 所以需要指明要控制的物理灯带，设备管理站点根据做转发
        */
    }
}



