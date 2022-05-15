//
// Created by 78472 on 2022/5/15.
//

#ifndef EXHIBITION_NONCOPYABLE_H
#define EXHIBITION_NONCOPYABLE_H


class noncopyable {
public:
    noncopyable(const noncopyable&) = delete;
    void operator=(const noncopyable&) = delete;

protected:
    noncopyable() = default;
    ~noncopyable() = default;
};


#endif //EXHIBITION_NONCOPYABLE_H
