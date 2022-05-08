//
// Created by 78472 on 2022/5/8.
//

#ifndef EXHIBITION_OBJECTHOLDER_H
#define EXHIBITION_OBJECTHOLDER_H


#include <unordered_map>
#include <mutex>
#include <functional>


using namespace std;


template<typename T>
class objectHolder {
private:
    std::unordered_map<const std::string, T *>  objectPtrMap;
    std::recursive_mutex mutex;

public:
    objectHolder() = default;
    ~objectHolder();

    void appendNew(const string& key, T* objPtr);

    T* findObject(const string& key);

    bool existObject(const string& key);

    void eraseObject(const string& key);

    using objectFunction = std::function<bool(const string& key, T*)>;
    void invokeOnAllObject(objectFunction func);

};




#endif //EXHIBITION_OBJECTHOLDER_H
