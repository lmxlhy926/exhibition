
#include <iostream>
#include <unordered_map>
#include <thread>
#include <chrono>


using namespace std;

int main(int argc, char* argv[]){
    std::unordered_map<string, string> umap;
    umap.insert(std::make_pair("hello", "first"));
    umap.insert(std::make_pair("hello", "second"));

    std::cout << umap.find("hello")->second << std::endl;

    while(true)
        std::this_thread::sleep_for(std::chrono::seconds(100));

    return 0;
}






















