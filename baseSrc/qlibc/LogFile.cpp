//
// Created by 78472 on 2022/5/31.
//

#include "LogFile.h"
#include <cstdio>
#include <cassert>


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

}


