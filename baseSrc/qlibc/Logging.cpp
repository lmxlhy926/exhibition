//
// Created by 78472 on 2022/5/30.
//

#include "Logging.h"
#include <iostream>
using namespace std;

namespace qlibc{
    void defaultOutput(const char* msg, size_t len, Logger::LogLevel level){
        switch(level){
            case Logger::LogLevel::INFO:
                fprintf(stdout, WHITE "%s" COLOR_NONE, msg);
                fflush(stdout);
                break;
            case Logger::LogLevel::HLIGHT:
                fprintf(stdout, DEEP_GREEN "%s" COLOR_NONE, msg);
                fflush(stdout);
                break;
        }
    }

    Logger::OutputFunc g_output = defaultOutput;

    Logger::Logger(const char* file, int line, qlibc::Logger::LogLevel level)
            : impl_(file, line, level){}

    Logger::~Logger() {
        impl_.finish();
        const LogStream::Buffer& buf(stream().buffer());
        g_output(buf.data(), buf.length(), impl_.level_);
    }

    void Logger::setOutput(qlibc::Logger::OutputFunc out) {
        g_output = out;
    }
}




