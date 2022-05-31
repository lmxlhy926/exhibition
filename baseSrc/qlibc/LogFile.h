//
// Created by 78472 on 2022/5/31.
//

#ifndef EXHIBITION_LOGFILE_H
#define EXHIBITION_LOGFILE_H

#include <string>
using namespace std;

namespace qlibc{
    //打开一个文件，将内容写入文件，
    class AppendFile{
    private:
        FILE* fp_;                      //文件描述符
        char buffer_[64 * 10224]{};     //缓冲区
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

}



#endif //EXHIBITION_LOGFILE_H
