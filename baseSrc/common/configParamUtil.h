//
// Created by 78472 on 2022/5/16.
//

#ifndef EXHIBITION_CONFIGPARAMUTIL_H
#define EXHIBITION_CONFIGPARAMUTIL_H

#include <string>
#include "qlibc/QData.h"

using namespace std;
using namespace qlibc;

class configParamUtil {
private:
    string dataDirPath;
    QData baseInfoData;
    QData recordData;
    QData secretFileNameData;
    QData mqttConfigData;
    QData interactiveAppData;
    static configParamUtil *instance;

private:
    explicit configParamUtil();

public:
    static configParamUtil *getInstance();

    void setConfigPath(const string &configPath);

    string getconfigPath();

    QData getBaseInfo();

    void saveBaseInfo(QData &data);

    QData getRecordData();

    void saveRecordData(QData &data);

    QData getSecretFileNameData();

    void saveSecretFileNameData(QData &data);

    QData getMqttConfigData();

    QData getInterActiveAppData();
};

#endif //EXHIBITION_CONFIGPARAMUTIL_H
