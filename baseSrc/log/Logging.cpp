//
// Created by 78472 on 2022/5/30.
//

#include "Logging.h"
#include <iostream>
#include <mutex>
#include <utility>

using namespace std;

namespace muduo{
    std::vector<spdlog::sink_ptr> sinks;
    std::shared_ptr<spdlog::logger> mylogger;
    void logInit(string& path) {
//        sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
        sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(path, 1024 * 1024 * 10, 1));
        mylogger = std::make_shared<spdlog::logger>("mylog", std::begin(sinks), std::end(sinks));
        mylogger->flush_on(spdlog::level::trace);   //设置出发err级别的消息时立即写入文件
        spdlog::register_logger(mylogger);
        spdlog::set_level(spdlog::level::trace);
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e][%s %# %!][thread %t][%l] : %v");
    }

    static std::recursive_mutex logging_output_mutex_;

    //这里没有使用length, 但是FixedBuffer的结构，保证msg一定是以'\0'结尾的
    static void defaultOutput(const char* msg, size_t length, Logger::LogLevel level){
        std::lock_guard<std::recursive_mutex> lg(logging_output_mutex_);
        switch(level){
            case Logger::LogLevel::H_WHITE:
                fprintf(stdout, WHITE "%s" COLOR_NONE, msg);
                break;
            case Logger::LogLevel::H_DEEP_GREEN:
                fprintf(stdout, DEEP_GREEN "%s" COLOR_NONE, msg);
                break;
            case Logger::LogLevel::H_RED:
                fprintf(stdout, RED "%s" COLOR_NONE, msg);
                break;
            case Logger::LogLevel::H_GREEN:
                fprintf(stdout, GREEN "%s" COLOR_NONE, msg);
                break;
            case Logger::LogLevel::H_YELLOW:
                fprintf(stdout, YELLOW "%s" COLOR_NONE, msg);
                break;
            case Logger::LogLevel::H_BLUE:
                fprintf(stdout, BLUE "%s" COLOR_NONE, msg);
                break;
            case Logger::LogLevel::H_PURPLE:
                fprintf(stdout, PURPLE "%s" COLOR_NONE, msg);
                break;
        }
        fflush(stdout);
    }

    static Logger::OutputFunc g_output = nullptr;

    Logger::Logger(const char* file, int line, const char* func, muduo::Logger::LogLevel level)
            : impl_(file, line, func, level){}

    Logger::~Logger() {
        impl_.finish();
        const LogStream::Buffer& buf(stream().buffer());
        if(impl_.level_ == LogLevel::H_RED){
            mylogger->error(string(buf.data(), buf.length() -1));
        }else{
            mylogger->info(string(buf.data(), buf.length() -1));
        }
        defaultOutput(buf.data(), buf.length(), impl_.level_);
        if(g_output != nullptr){
            g_output(buf.data(), buf.length(), impl_.level_);
        }
    }

    void Logger::setOutput(muduo::Logger::OutputFunc out) {
        g_output = std::move(out);
    }
}




