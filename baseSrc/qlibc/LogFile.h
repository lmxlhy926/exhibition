//
// Created by 78472 on 2022/5/31.
//

#ifndef EXHIBITION_LOGFILE_H
#define EXHIBITION_LOGFILE_H

#include <string>
#include <memory>
#include <mutex>
#include <cstdlib>
#include <ctime>

using namespace std;

namespace qlibc{
    //打开一个文件，将内容写入文件，
    class AppendFile{
    private:
        FILE* fp_;                      //文件描述符
        char buffer_[64 * 1024]{};      //缓冲区
        size_t writtenBytes_;           //写入文件的字节总数
    private:
        size_t write(const char* logline, size_t len);
    public:
        explicit AppendFile(const string& fileName);

        ~AppendFile();

        //将内容写入文件
        void append(const char* logline, size_t len);

        //文件刷新
        void flush();

        size_t writtenBytes() const { return writtenBytes_;}
    };

    /*
     * 将日志写入到日志文件：
     *      日志文件会自动更新：到达文件设定的可写入的最大字节数，或者日期天数发生变化
     */
    class LogFile{
    public:
        LogFile(const string& baseName,
                size_t rollSize,
                bool threadSafe = true,
                int flushInterval = 3,
                int checkEveryN = 1024);

        ~LogFile() = default;

        //写入打印信息
        void append(const char* logline, size_t len);
        //刷新内容到文件中
        void flush();

    private:
        //生成新的日志文件
        bool rollFile();

        void append_unlocked(const char* logline, int len);

        //生成组装的日志文件名
        static string getLogFileName(const string& baseName, time_t* now);

        const string baseName_;     //文件基础名字
        const size_t rollSize_;     //文件的最大容纳字节数
        std::unique_ptr<std::recursive_mutex> mutex_;   //互斥锁
        const int flushInterval_;   //最大刷新间隔
        const int checkEveryN_;     //写入checkEveryN_次后，检查是否需要刷新以及更新日志文件

        int count_;                 //记录写入次数，计数到checkEveryN_后归零一次

        time_t startOfPeriod_;      //创建日志文件的日期：精确到天数
        time_t lastRoll_;           //上一次产生新的日志文件的时间戳
        time_t lastFlush_;          //上一次刷新操作的时间戳

        std::unique_ptr<AppendFile> file_;

        const static int kRollPerSecods = 60 * 60 * 24;     //一天代表的秒数
    };


}



#endif //EXHIBITION_LOGFILE_H
