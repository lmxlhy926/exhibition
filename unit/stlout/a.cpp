#include <iostream>
#include <string>
#include "qlibc/QData.h"
using namespace std;

void test(){
    qlibc::QData localData;
    try{
        long long int localTime = stoull(localData.getString("timeStamp"), nullptr, 10);
        std::cout << localTime << std::endl;
    }catch(const exception& e){
        std::cout << e.what() << std::endl;
        std::cout << "--------" << std::endl;
    }
   

}

void cmdLinePrint(int argc, char* argv[]){
    std::cout << "argc: " << argc << std::endl;
    for(int i = 0; i < argc; ++i){
        printf("%s\n", argv[i]);
    }
}

int main(int argc, char* argv[]){
    test();

    return 0;
}











