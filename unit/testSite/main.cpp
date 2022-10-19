#include <iostream>
#include <string>
#include <algorithm>
#include <vector>


using namespace std;

string hump2Underline(string in) {
    string out;
    for(int i = 0; i < in.size(); ++i)
    {
        if (in[i] <= 'Z' && in[i] >= 'A') {
            if(i == 0){
                out.push_back(in[i] - 'A' + 'a');
            }else{
                out.push_back('_');
                out.push_back(in[i] - 'A' + 'a');
            }
        }else {
            out.push_back(in[i]);
        }
    }
    return out;
}

string toUpper(string in) {
    transform(in.begin(), in.end(), in.begin(), ::toupper);
    return in;
}

int main() {
    std::vector<string> vec{
        "airCleaner",
        "humidifier"
    };
    for(auto& elem : vec){
        std::cout << toUpper(hump2Underline(elem)) << std::endl;
    }

    return 0;
}
