
#include "log/Logging.h"
#include "common/httplib.h"
#include <fstream>
#include "qlibc/QData.h"

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



int main(int argc, char* argv[]){

//    threadLogTest();
//    test();

    LOG_INFO << "1";
    LOG_RED << "1";
    LOG_YELLOW << "1";
    LOG_GREEN << "1";


    return 0;
}
