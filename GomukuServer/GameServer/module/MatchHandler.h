#pragma once

#include <mutex>
#include <vector>
#include <list>
#include <thread>
#include <condition_variable>

#include <jsoncpp/json/json.h>

#include "../MyMuduo/TcpServer.h"

//匹配池
class MatchPool
{
public:
    MatchPool();
    ~MatchPool() = default;

    bool addUser(const uint32_t &userId);
    bool removeUser(const uint32_t &userId);

private:
    void matchThread();

private:
    std::mutex m_mutex;
    std::condition_variable m_cond;
    std::thread m_matchThread;                    // 匹配线程
    std::vector<std::list<uint32_t>> m_matchPool; // list存的是用户id
};

// 匹配模块协议处理入口
class MatchHandler
{
public:
    MatchHandler();

    void onMatch(const TcpConnectionPtr &conn, const Json::Value &req);       // 处理匹配请求
    void onCancelMatch(const TcpConnectionPtr &conn, const Json::Value &req); // 处理取消匹配请求

private:
    MatchPool m_matchPool;
};

// 全局初始化：确保启动时注册协议
extern MatchHandler g_MatchHandler;