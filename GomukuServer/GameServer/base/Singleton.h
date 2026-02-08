#pragma once

#include "NonCopyable.h"
#include <mutex>
#include <memory>

template <typename T>
class Singleton : NonCopyable
{
public:
    // 全局获取单例实例
    static T &getInstance()
    {
        // C++11局部静态变量线程安全，懒汉式初始化
        static std::unique_ptr<T> instance(new T());
        return *instance;
    }

private:
    Singleton() = default;
    ~Singleton() = default;
};