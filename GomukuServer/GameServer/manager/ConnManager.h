#pragma once

#include <unordered_set>
#include <mutex>

#include <jsoncpp/json/json.h>

#include "../MyMuduo/TcpServer.h"

#include "../base/NonCopyable.h"
#include "../base/Singleton.h"

class ConnManager : NonCopyable
{
    friend class Singleton<ConnManager>;

public:
    // 添加连接
    void addConn(const TcpConnectionPtr &conn);                                              // 添加连接
    void removeConn(const TcpConnectionPtr &conn);                                           // 移除连接（主动关闭/断连）
    bool isConnValid(const TcpConnectionPtr &conn);                                          // 检查连接是否有效
    void sendMsg(const TcpConnectionPtr &conn, const int &protocol, const Json::Value &msg); // 向指定连接发送消息

private:
    ConnManager();

    std::mutex m_mutex;
    std::unordered_set<TcpConnectionPtr> m_conns; // 存储所有有效连接
};

// 全局快捷调用
#define g_ConnMgr Singleton<ConnManager>::getInstance()