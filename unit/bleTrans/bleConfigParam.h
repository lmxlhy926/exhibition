//
// Created by 78472 on 2022/6/7.
//

#ifndef EXHIBITION_BLECONFIGPARAM_H
#define EXHIBITION_BLECONFIGPARAM_H

#include "qlibc/QData.h"
using namespace qlibc;

class bleConfigParam {
private:
    string dataDirPath;
    QData bleParamData;
    static bleConfigParam* instance;
    std::recursive_mutex mutex_;

private:
    bleConfigParam() = default;

public:
    static bleConfigParam* getInstance();

    void setConfigPath(const string &configPath);

    string getconfigPath();

    QData getBleParamData();

};


#endif //EXHIBITION_BLECONFIGPARAM_H
