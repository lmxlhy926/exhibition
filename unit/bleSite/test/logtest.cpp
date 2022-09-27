
#include "log/Logging.h"
#include "common/httplib.h"
#include <fstream>
#include "qlibc/QData.h"
#include <ctime>
#include <sys/time.h>
#include "formatTrans/downUtil.h"


void allTypeTest(){

    LOG_INFO << "true: "<< true;
    LOG_INFO << "false: " << false;

    LOG_RED << "short: " << static_cast<short>(100);
    LOG_RED << "unsigned short: " << static_cast<unsigned short>(100);
    LOG_RED << "int: " << static_cast<int>(100);
    LOG_YELLOW << "unsigned int: " << static_cast<unsigned int>(100);
    LOG_YELLOW << "long: " << static_cast<long>(100);
    LOG_YELLOW << "unsigned long: " << static_cast<unsigned long>(100);
    LOG_BLUE << "long long: " << static_cast<long long>(100);
    LOG_BLUE << "unsigned long long: " << static_cast<unsigned long long>(100);

    LOG_GREEN << "float: " << static_cast<float>(2.53);
    LOG_GREEN << "double: " << static_cast<double>(2.53);

    char a[10] = {1, 2, 3};
    LOG_PURPLE << "&a[0]: " << reinterpret_cast<const void*>(&a[0]);
    LOG_PURPLE << "&a[1]: " << reinterpret_cast<const void*>(&a[1]);
    LOG_PURPLE << "&a[2]: " << reinterpret_cast<const void*>(&a[2]);
    LOG_HLIGHT << "&a[3]: " << reinterpret_cast<const void*>(&a[3]);
    LOG_HLIGHT << "&a[4]: " << reinterpret_cast<const void*>(&a[4]);

    string str = "welcome to the world";
    LOG_HLIGHT << str;
}

void threadLogTest(){


    httplib::ThreadPool threadPool(10);


    threadPool.enqueue([](){
        for(int i = 0; i < 3000; i++){
            allTypeTest();
        }
    });

    threadPool.enqueue([](){
        for(int i = 0; i < 3000; i++){
            allTypeTest();
        }
    });

    threadPool.enqueue([](){
        for(int i = 0; i < 30000; i++){
            allTypeTest();
        }
    });

    threadPool.shutdown();
}


void test(){
    qlibc::QData f;
    f.loadFromFile(R"(D:\bywg\project\exhibition\unit\bleSite\test\test.json)");
    LOG_INFO << f.toJsonString();

}

void printColor(){
    LOG_INFO << "INFO";
    LOG_GREEN << "GREEN";
    LOG_HLIGHT << "HLIGHT";
    LOG_BLUE << "BLUE";
    LOG_RED << "RED";
    LOG_YELLOW << "YELLOW";

}

void test1(){
    struct timeval tv;
    struct timezone tz;
    struct tm *t;

    gettimeofday(&tv, &tz);
    t = localtime(&tv.tv_sec);
    printf("time_now: %d-%d-%d %d:%d:%d.%ld\n", 1900 + t->tm_year, 1 + t->tm_mon, t->tm_mday,
           t->tm_hour, t->tm_min, t->tm_sec, tv.tv_usec);
    printColor();
}


int main(int argc, char* argv[]){
    std::cout << LightGatewayAddressAssign().getBinaryString();


    return 0;
}
