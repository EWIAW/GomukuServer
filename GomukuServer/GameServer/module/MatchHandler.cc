#include "MatchHandler.h"

#include "../base/Log.h"

#include "../manager/UserManager.h"

MatchPool::MatchPool()
{
    // 初始化匹配队列，一个三条匹配队列，对于三种分段，0-2000分，2000-4000分，4000+分
    for (int i = 0; i < 3; i++)
    {
        m_matchPool.push_back(std::list<uint32_t>());
    }

    // 创建专门的匹配线程
    m_matchThread = std::thread(std::bind(&MatchPool::matchThread, this));
    m_matchThread.detach();
}

bool MatchPool::addUser(const uint32_t &userId)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    std::shared_ptr<UserInfo> user = g_UserMgr.getUserById(userId);
    if (user != nullptr && user->getState() != UserState::ONLINE)
    {
        return false;
    }

    if (user->getUserPoints() < 2000)
    {
        m_matchPool[0].push_back(userId);
    }
    else if (user->getUserPoints() < 4000)
    {
        m_matchPool[1].push_back(userId);
    }
    else
    {
        m_matchPool[2].push_back(userId);
    }
    user->setState(UserState::MATCHING);
    m_cond.notify_one();
    DLOG("用户 %d 加入匹配队列成功", userId);
    return true;
}

bool MatchPool::removeUser(const uint32_t &userId)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    std::shared_ptr<UserInfo> user = g_UserMgr.getUserById(userId);
    if (user != nullptr && user->getState() != UserState::MATCHING)
    {
        return false;
    }

    if (user->getUserPoints() < 2000)
    {
        m_matchPool[0].remove(userId);
    }
    else if (user->getUserPoints() < 4000)
    {
        m_matchPool[1].remove(userId);
    }
    else
    {
        m_matchPool[2].remove(userId);
    }
    user->setState(UserState::ONLINE);
    DLOG("用户 %d 退出匹配队列成功", userId);
    return true;
}

void MatchPool::matchThread()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    while (true)
    {
        while (m_matchPool[0].size() <= 1 && m_matchPool[1].size() <= 1 && m_matchPool[2].size() <= 1)
        {
            m_cond.wait(lock);
        }

        for (int i = 0; i < 3; i++)
        {
            if (m_matchPool[i].size() >= 2)
            {
                uint32_t userId1 = m_matchPool[i].front(); 
                m_matchPool[i].pop_front();
                uint32_t userId2 = m_matchPool[i].front();
                m_matchPool[i].pop_front();

                //为他们创建一个游戏房间
            } 
        }
    }
}
