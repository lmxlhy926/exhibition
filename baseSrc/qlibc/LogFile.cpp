//
// Created by 78472 on 2022/5/31.
//

#include "LogFile.h"
#include <cstdio>
#include <cassert>
#include <ctime>
#include <iostream>


namespace qlibc{
    size_t AppendFile::write(const char *logline, size_t len) {
        return fwrite(logline, 1, len, fp_);
    }

    //打开文件，设置缓存buffer
    AppendFile::AppendFile(const string &fileName)
        : fp_(fopen(fileName.c_str(), "ae")), writtenBytes_(0)
    {
        assert(fp_);    //不能打开文件则终止程序
        ::setbuffer(fp_, buffer_, sizeof buffer_);
    }

    //析构时关闭文件
    AppendFile::~AppendFile() {
        ::fclose(fp_);
    }

    //文件写入，增加写入文件的字节数
    void AppendFile::append(const char *logline, size_t len) {
        size_t written = 0;
        while(written != len){
            size_t remain = len - written;
            size_t n = write(logline, remain);
            if(n != remain){
                int err = ferror(fp_);
                if(err){
                    fprintf(stderr, "AppendFile::append() failed\n");
                    break;
                }
            }
            written += n;
        }

        writtenBytes_ += written;
    }

    //刷新文件
    void AppendFile::flush() {
        ::fflush(fp_);
    }


    LogFile::LogFile(const string &baseName,
                     size_t rollSize,
                     bool threadSafe,
                     int flushInterval,
                     int checkEveryN)
    :   baseName_(baseName),
        rollSize_(rollSize),
        mutex_(threadSafe ? new std::recursive_mutex : nullptr),
        flushInterval_(flushInterval),
        checkEveryN_(checkEveryN),
        startOfPeriod_(0),
        lastRoll_(0),
        lastFlush_(0)
    {
        rollFile();
    }

    void LogFile::append(const char *logline, size_t len) {
        if(mutex_ != nullptr){
            std::lock_guard<std::recursive_mutex> lg(*mutex_);
            append_unlocked(logline, len);
        }else{
            append_unlocked(logline, len);
        }
    }

    void LogFile::flush() {
        if(mutex_ != nullptr){
            std::lock_guard<std::recursive_mutex> lg(*mutex_);
            file_->flush();
        }else{
            file_->flush();
        }
    }

    bool LogFile::rollFile() {
        time_t now = 0;
        string fileName = getLogFileName(baseName_, &now);
        time_t start = now / kRollPerSecods * kRollPerSecods;   //换算成整天的秒数

        if(now > lastRoll_){
            lastRoll_ = now;
            lastFlush_ = now;
            startOfPeriod_ = start;     //日志文件创建的当前日期
            file_.reset(new AppendFile(fileName));
            return true;
        }
        return false;
    }

    void LogFile::append_unlocked(const char *logline, int len) {
        //第一步：将内容写入到文件中
        file_->append(logline, len);

        /*
         *  1. 如果超过文件的最大限定字符数, 则生成新的日志文件
         *  2. 写入次数超过checkEveryN_:
         *          如果距日志创建日期过了一天，则生成新的日志文件
         *          如果超过了最大刷新时间，则进行一次刷新
         */
        if(file_->writtenBytes() > rollSize_){
            std::cout << "----writtenBytes: " << file_->writtenBytes() << std::endl;
            rollFile();
        }else{
            ++count_;
            if(count_ >= checkEveryN_){
                count_ = 0;
                time_t now = ::time(nullptr);
                time_t thisPeriod = now / kRollPerSecods * kRollPerSecods;
                if(thisPeriod != startOfPeriod_){
                    rollFile();
                }else if(now - lastFlush_ > flushInterval_){
                    lastFlush_ = now;
                    file_->flush();
                }
            }
        }
    }

    string LogFile::getLogFileName(const string &baseName, time_t *now) {
        string fileName;
        fileName.reserve(baseName.size() + 64);
        fileName = baseName;

        char timebuf[32];
        struct tm tm{};
        *now = time(nullptr);
        gmtime_r(now, &tm);
        strftime(timebuf, sizeof timebuf, ".%Y%m%d-%H%M%S", &tm);
        fileName += timebuf;
        fileName += ".log";

        return fileName;
    }


}


