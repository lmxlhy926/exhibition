//
// Created by 78472 on 2022/5/30.
//

#include "Logging.h"
#include <iostream>
#include <mutex>
#include <utility>
using namespace std;

namespace qlibc{

    static std::recursive_mutex* logging_output_mutex_ = new recursive_mutex();

    static void defaultOutput(const char* msg, size_t len, Logger::LogLevel level){
        std::lock_guard<std::recursive_mutex> lg(*logging_output_mutex_);
        switch(level){
            case Logger::LogLevel::INFO:
                fprintf(stdout, WHITE "%s" COLOR_NONE, msg);
                break;
            case Logger::LogLevel::HLIGHT:
                fprintf(stdout, DEEP_GREEN "%s" COLOR_NONE, msg);
                break;
        }
        fflush(stdout);
    }

    static Logger::OutputFunc g_output = nullptr;

    Logger::Logger(const char* file, int line, qlibc::Logger::LogLevel level)
            : impl_(file, line, level){}

    Logger::~Logger() {
        impl_.finish();
        const LogStream::Buffer& buf(stream().buffer());
        defaultOutput(buf.data(), buf.length(), impl_.level_);
        if(g_output != nullptr){
            g_output(buf.data(), buf.length(), impl_.level_);
        }
    }

    void Logger::setOutput(qlibc::Logger::OutputFunc out) {
        g_output = std::move(out);
    }
}




