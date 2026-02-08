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
    m_userId2user[user->getUserId()] = userPtr;
    if (user->getConn())
    {
        m_conn2UserId[user->getConn()] = user->getUserId();
    }
    DLOG("用户上线id : %u, 用户名 : %s", user->getUserId(), user->getUserName().c_str());
}

// 移除在线玩家（下线/断连）
void UserManager::removeOnlineUser(uint32_t userId)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_userId2user.find(userId);
    if (it != m_userId2user.end())
    {
        if (it->second->getConn())
        {
            m_conn2UserId.erase(it->second->getConn());
        }
        m_userId2user.erase(it);
        DLOG("用户下线id : %u", userId);
    }
}

// 根据玩家ID获取玩家（返回nullptr则不存在）
std::shared_ptr<UserInfo> UserManager::getUserById(uint32_t userId)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_userId2user.find(userId);
    return it != m_userId2user.end() ? it->second : nullptr;
}

// 根据网络连接获取玩家ID（0则不存在）
uint32_t UserManager::getUserIdByConn(const TcpConnectionPtr &conn)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_conn2UserId.find(conn);
    return it != m_conn2UserId.end() ? it->second : 0;
}

// // 检查玩家是否在线且状态为指定状态
// bool UserManager::checkUserState(uint32_t userId, UserState state)
// {
//     UserInfo *user = getUserById(userId);
//     return user && user->getState() == state;
// }

// // 更新玩家状态
// bool UserManager::updateUserState(uint32_t userId, UserState state)
// {
//     UserInfo *user = getUserById(userId);
//     if (user)
//     {
//         user->setState(state);
//         return true;
//     }
//     return false;
// }