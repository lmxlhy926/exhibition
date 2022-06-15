//
// Created by 78472 on 2022/6/15.
//

#ifndef EXHIBITION_CONTROLCMD_H
#define EXHIBITION_CONTROLCMD_H

#include <string>
#include "qlibc/QData.h"
#include "jsonCmd2Binary.h"

using namespace std;

class LightScanAddDel : public JsonCmd2Binary{
private:
    string pseudoCommand;
    string device_id;
public:
    explicit LightScanAddDel(qlibc::QData& data){ init(data); }

    void init(qlibc::QData& data);

    size_t getBinary(unsigned char* buf, size_t bufSize) override;
};


class LightOnOff : public JsonCmd2Binary{
private:
    string pseudoCommand;
    string address;
public:
    explicit LightOnOff(qlibc::QData& data){ init(data); }

    void init(qlibc::QData& data);

    size_t getBinary(unsigned char* buf, size_t bufSize) override;
};



#endif //EXHIBITION_CONTROLCMD_H
