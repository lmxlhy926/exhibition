//
// Created by 78472 on 2022/5/11.
//

#ifndef EXHIBITION_CONFIGPARAMUTIL_H
#define EXHIBITION_CONFIGPARAMUTIL_H

#include <string>

using namespace std;

class configParamUtil {
private:
    string dataDirPath;
    static configParamUtil* instance;

private:
    explicit configParamUtil();

public:
    static configParamUtil* getInstance();

    void setConfigPath(const string& configPath);

    string getconfigPath();

};


#endif //EXHIBITION_CONFIGPARAMUTIL_H
