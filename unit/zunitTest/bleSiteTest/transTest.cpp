
#include <iostream>
#include <thread>
#include <chrono>
#include "jsonCmd2Binary.h"
#include "binary2JsonEvent.h"

void binaryString2JsonEvent(){
    string binaryString_lightOnOff = "0c0091810027002600820401000a";
    string binaryString_brightness = "0B00918100131FFFFF824EFFFF";

    BinaryString2JsonEvent bsLightOnOff(binaryString_lightOnOff);
    std::cout << "binaryString_lightOnOff: " << bsLightOnOff.getJsonStringEvent() << std::endl;

    BinaryString2JsonEvent bsBrightness(binaryString_brightness);
    std::cout << "binaryString_brightness: " << bsBrightness.getJsonStringEvent() << std::endl;
}

void binary2JsonEvent(string& binaryString){
    //转换为二进制格式
    unsigned char buf[100];
    size_t bufSize = JsonCmd2Binary::binaryString2binary(binaryString, buf, 100);
    std::cout << "bufSize: " << bufSize << std::endl;
    for(size_t i = 0; i < bufSize; i++){
        printf("==>%2x\n", buf[i]);
    }

    //二进制转换为字符串格式命令
    string tranString = CharArray2BinaryString::getBinaryString(buf, bufSize);
    std::cout << "tranString: " << tranString << std::endl;

    BinaryString2JsonEvent bje(tranString);
    std::cout << "jsonEvent: " << bje.getJsonStringEvent() << std::endl;
}


int main(int argc, char* argv[]){
    string binaryString_lightOnOff = "0c0091810027002600820401000a";
    string binaryString_brightness = "0B00918100131FFFFF824EFFFF";

    binary2JsonEvent(binaryString_lightOnOff);
    binary2JsonEvent(binaryString_brightness);

    while(true){
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    return 0;
}



















