#include "UserManager.h"

#include "../base/Log.h"

UserManager::UserManager() // 私有构造：单例模式
{
    DLOG("初始化UserManager单例");
}

// 添加在线玩家
void UserManager::addOnlineUser(UserInfo *user)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    std::shared_ptr<UserInfo> userPtr(user);
    m_UserId2Conn[userPtr->getUserId()] = userPtr->getConn();
    m_Conn2UserId[user->getConn()] = user->getUserId();
    m_UserId2User[user->getUserId()] = userPtr;
    DLOG("用户上线id : %u, 用户名 : %s", user->getUserId(), user->getUserName().c_str());
}

// 移除在线玩家（下线/断连）
void UserManager::removeOnlineUser(uint32_t userId)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_UserId2Conn.find(userId);
    if (it != m_UserId2Conn.end())
    {
        m_UserId2Conn.erase(userId);
        m_Conn2UserId.erase(it->second);
        m_UserId2User.erase(userId);
        DLOG("用户下线id : %u", userId);
    }
}

// 根据玩家ID获取玩家（返回nullptr则不存在）
std::shared_ptr<UserInfo> UserManager::getUserByUserId(uint32_t userId)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_UserId2User.find(userId);
    return it != m_UserId2User.end() ? it->second : nullptr;
}

// 根据玩家tcpconn获取玩家
std::shared_ptr<UserInfo> UserManager::getUserByconn(const TcpConnectionPtr &conn)
{
    // 先通过玩家的conn获取玩家的id
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_Conn2UserId.find(conn);
    if (it == m_Conn2UserId.end())
    {
        return nullptr;
    }

    // 在通过玩家的id获取用户信息
    auto it2 = m_UserId2User.find(it->second);
    if (it2 == m_UserId2User.end())
    {
        return nullptr;
    }
    return it2->second;
}

// 根据玩家连接获取玩家的ID
uint32_t UserManager::getUserIdByConn(const TcpConnectionPtr &conn)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_Conn2UserId.find(conn);
    return it != m_Conn2UserId.end() ? it->second : 0;
}

// 根据玩家ID获取玩家的连接
TcpConnectionPtr UserManager::getConnByUserId(uint32_t userId)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_UserId2Conn.find(userId);
    return it != m_UserId2Conn.end() ? it->second : nullptr;
}
