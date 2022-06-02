//
// Created by 78472 on 2022/6/2.
//

#ifndef EXHIBITION_CONTRETEDEVICECONTROL_H
#define EXHIBITION_CONTRETEDEVICECONTROL_H

#include "qlibc/QData.h"
#include "common.h"

using namespace std;


class CommonControl : public ControlBase{
public:
    void operator()(const DownCommandData& downCommand, qlibc::QData& deviceList);

private:
    bool match(const DownCommandData &downCommand, qlibc::QData &deviceItem) override;

    qlibc::QData constructCtrCmd(const DownCommandData &downCommand, qlibc::QData &deviceItem) override;
};


#endif //EXHIBITION_CONTRETEDEVICECONTROL_H
