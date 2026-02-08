#pragma once

#include <mutex>
#include <memory>
#include <unordered_map>

#include "../base/NonCopyable.h"
#include "../base/Singleton.h"

#include "../model/UserInfo.h"

// 全局在线玩家管理器：单例，线程安全，所有模块通过此管理器操作玩家
class UserManager : NonCopyable
{
    friend class Singleton<UserManager>;

public:
    void addOnlineUser(UserInfo *user);                     // 添加在线玩家
    void removeOnlineUser(uint32_t userId);                 // 移除在线玩家（下线/断连）
    std::shared_ptr<UserInfo> getUserById(uint32_t userId); // 根据玩家ID获取玩家（返回nullptr则不存在）
    uint32_t getUserIdByConn(const TcpConnectionPtr &conn); // 根据网络连接获取玩家ID（0则不存在）
    // bool checkUserState(uint32_t userId, UserState state);  // 检查玩家是否在线且状态为指定状态
    // bool updateUserState(uint32_t userId, UserState state); // 更新玩家状态

private:
    UserManager(); // 私有构造：单例模式

    std::mutex m_mutex;                                                    // 互斥锁：保证多线程安全
    std::unordered_map<uint32_t, std::shared_ptr<UserInfo>> m_userId2user; // 玩家ID → 玩家对象
    std::unordered_map<TcpConnectionPtr, uint32_t> m_conn2UserId;          // 连接 → 玩家ID
};

// 全局快捷调用
#define g_UserMgr Singleton<UserManager>::getInstance()