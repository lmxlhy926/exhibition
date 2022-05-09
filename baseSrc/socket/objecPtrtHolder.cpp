//
// Created by 78472 on 2022/5/8.
//

#include "objecPtrtHolder.h"

template<typename T>
objectPtrHolder<T>::~objectPtrHolder() {
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    for(auto& elem : objectPtrMap){
        delete objectPtrMap->second;
    }
}

template<typename T>
void objectPtrHolder<T>::appendNew(const string &key, T *objPtr) {
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    if(objPtr == nullptr || key.empty())
        return;
    objectPtrMap.insert(std::make_pair(key, objPtr));
}

template<typename T>
T *objectPtrHolder<T>::findObject(const string &key) {
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    if(key.empty())
        return nullptr;
    auto ret = objectPtrMap.find(key);
    if(ret != objectPtrMap.end())
        return ret->second;
    else
        return nullptr;
}

template<typename T>
bool objectPtrHolder<T>::existObject(const string &key) {
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    if(key.empty())
        return false;
    auto ret = objectPtrMap.find(key);
    if(ret == objectPtrMap.end())
        return false;
    else
        return true;
}

template<typename T>
void objectPtrHolder<T>::eraseObject(const string &key) {
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    if(key.empty())
        return;
    auto ret = objectPtrMap.find(key);
    if(ret == objectPtrMap.end())
        return;
    else{
        delete ret->second;
        objectPtrMap.erase(key);
        return;
    }
}

template<typename T>
void objectPtrHolder<T>::invokeOnAllObject(objectPtrHolder::objectFunction func) {
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    for(auto & elem : objectPtrMap){
        func(elem->first, elem->second);
    }
}