//
// Created by 78472 on 2022/5/30.
//

#ifndef EXHIBITION_LOGGING_H
#define EXHIBITION_LOGGING_H

#include "LogStream.h"
#include <functional>

/*
带格式控制的一般格式为：
    "\033[控制码1； 控制码2；控制码3...m字符串内容\033[控制码m"
控制码：
    \033[0m    关闭所有属性   ：一般放在后面，这样只会影响你输入的字符串的格式
    \033[1m    设置高亮度
    \033[4m     下划线
    \033[5m    闪烁
    \033[7m    反显
    \033[8m    消隐
    \033[30m    --    \033[37m    设置字体颜色
    \033[40m    --    \033[47m    设置背景色

字背景颜色范围:
    40  41  42  43  44   45   46   47
    黑  红  绿   黄   蓝   紫   深绿  白
字体颜色范围：
    30  31  32  33  34   35   36   37
    黑  红  绿   黄   蓝   紫   深绿  白
 */

#define COLOR_NONE  "\033[0m"        //表示清除前面设置的格式
#define BLACK       "\033[1;30m"     //黑色
#define RED         "\033[1;31m"     //红色
#define GREEN       "\033[1;32m"     //绿色
#define YELLOW      "\033[1;33m"     //黄色
#define BLUE        "\033[1;34m"     //蓝色
#define PURPLE      "\033[1;35m"     //紫色
#define DEEP_GREEN   "\033[1;36m"     //深绿
#define WHITE       "\033[1;37m"     //白色


namespace qlibc{

    class Logger {
    public:
        enum LogLevel{
            INFO,
            HLIGHT
        };

        explicit Logger(const char* file, int line, LogLevel level);
        ~Logger();

        LogStream& stream() { return impl_.stream_; }

        using OutputFunc = std::function<void(const char* msg, size_t len, Logger::LogLevel level)>;
        static void setOutput(OutputFunc);

    private:
        //__FILE__是绝对路径，从中提取出文件名
        class SourceFile{
        private:
            const char* data_;
            size_t size_;
        public:
            explicit SourceFile(const char* fileName) : data_(fileName){
                const char* slash = strrchr(data_, '/');
                if(slash){
                    data_ = slash + 1;
                }
                size_ = strlen(data_);
            }

            string data(){
                return string(data_, size_);
            }
        };

        class Impl{
        public:
            SourceFile fileName_;
            int line_;
            Logger::LogLevel level_;
            LogStream stream_;

        public:
            explicit Impl(const char* fileName, int line, Logger::LogLevel level)
                : fileName_(fileName), line_(line), level_(level){}

            void finish(){
                stream_ << " - " << fileName_.data() << ":" << line_ << "\n";
            }
        };

        Impl impl_;
    };

    #define LOG_INFO qlibc::Logger(__FILE__, __LINE__, qlibc::Logger::INFO).stream()
    #define LOG_HLIGHT qlibc::Logger(__FILE__, __LINE__, qlibc::Logger::HLIGHT).stream()

}




#endif //EXHIBITION_LOGGING_H
