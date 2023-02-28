#include "log/Logging.h"
#include <string>
using namespace std;

int main(int argc, char* argv[]){

    string path = "/data/changhong/edge_midware/light_move/log.txt";
    muduo::logInit(path);
    LOG_GREEN << "HELLO";

    return 0;
}
