#include <iostream>
#include <fstream>
#include <regex>
#include <set>
#include <functional>

#include <fcntl.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <unistd.h>


using namespace std;

int main(int argc, char* argv[]){
    std::set<string> panelAddressSet;
    FILE* queryLogFilePtr = fdopen(STDIN_FILENO, "r");;
    if(queryLogFilePtr != nullptr){
        while(true){
            char buf[1024]{};
            if(fgets(buf, 1024, queryLogFilePtr) != nullptr){
                string line{buf};
                regex addressReg("([0-9]*[.][0-9]*[.][0-9]*[.][0-9]*)");
                sregex_iterator begin(line.cbegin(), line.cend(), addressReg);
                sregex_iterator end;
                for(; begin != end; ++begin){
                     panelAddressSet.insert(begin->str(1));
                }
            }else{
                break;
            }
        }
    }
    copy(panelAddressSet.begin(), panelAddressSet.end(), ostream_iterator<string>(cout, " "));
    return 0;
}