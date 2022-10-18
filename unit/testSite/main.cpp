#include <iostream>
#include <string>
using namespace std;

int main() {
    string in = "airCleanMachine";
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
    cout << out << endl;

}
