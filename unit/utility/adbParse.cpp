#include <iostream>
#include <fstream>
#include <regex>
#include <set>

#include <fcntl.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <unistd.h>


using namespace std;

int main(int argc, char* argv[]){
    FILE* queryLogFilePtr = nullptr;
    string option, fileName;
    std::set<string> panelAddressSet;

    if(argc == 1){
        option.assign("host");
        queryLogFilePtr = fdopen(STDIN_FILENO, "r");

    }else if(argc == 2){
        option.assign(argv[1]);
        queryLogFilePtr = fdopen(STDIN_FILENO, "r");

    }else if(argc == 3){
        option.assign(argv[1]);
        fileName.assign(argv[2]);
        queryLogFilePtr = fopen(fileName.c_str(), "r");
    }

    if(queryLogFilePtr != nullptr){
        char buf[1024];
        while(true){
            if(fgets(buf, 1024, queryLogFilePtr) != nullptr){
                string line{buf};
                smatch sm;
                if(regex_search(line, sm, regex("(.*):[0-9]{4}.*"))){
                    if(option == "host"){
                        panelAddressSet.insert(sm.str(1));
                    }else{
                        panelAddressSet.insert(sm.str(1) + ":5353");
                    }
                }
            }else{
                break;
            }
        }
    }

    for(auto& elem : panelAddressSet){
        std::cout << elem << " ";
    }

    return 0;
}