#include <iostream>

int main(int argc, char* argv[]){
    std::cout << "argc: " << argc << std::endl;
    for(int i = 0; i < argc; ++i){
        printf("%s\n", argv[i]);
    }

    return 0;
}











