#include "MatchHandler.h"

#include "../base/Log.h"
#include "../base/ProtocolId.h"

#include "../manager/ConnManager.h"
#include "../manager/GameRoomManager.h"
#include "../manager/UserManager.h"

#include "../net/ProtocolDispatcher.h"

MatchHandler g_MatchHandler;

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
    std::shared_ptr<UserInfo> user = g_UserMgr.getUserByUserId(userId);
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
    std::shared_ptr<UserInfo> user = g_UserMgr.getUserByUserId(userId);
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

                // 为他们创建一个游戏房间
                TcpConnectionPtr conn1 = g_UserMgr.getConnByUserId(userId1);
                TcpConnectionPtr conn2 = g_UserMgr.getConnByUserId(userId2);
                std::shared_ptr<UserInfo> user1 = g_UserMgr.getUserByconn(conn1);
                std::shared_ptr<UserInfo> user2 = g_UserMgr.getUserByconn(conn2);
                Json::Value res;

                // 1.创建房间之前，先判断是否有人掉线
                if (conn1 == nullptr || conn2 == nullptr)
                {
                    res["success"] = false;
                    res["reason"] = "对方掉线，匹配失败";
                    if (conn1 == nullptr)
                    {
                        user2->setState(UserState::ONLINE);
                        g_ConnMgr.sendMsg(conn2, ProtocolId::MATCH_PUSH, res);
                    }
                    else
                    {
                        user1->setState(UserState::ONLINE);
                        g_ConnMgr.sendMsg(conn1, ProtocolId::MATCH_PUSH, res);
                    }
                }

                // 2.没人掉线，则正常为他们创建房间
                g_GameRoomMgr.createRoom(userId1, userId2);
                user1->setState(UserState::GAMING);
                user2->setState(UserState::GAMING);
                res["success"] = true;
                res["reason"] = "匹配成功，准备进入游戏房间";
                res["color"] = 1;
                g_ConnMgr.sendMsg(conn1, ProtocolId::MATCH_PUSH, res);
                res["color"] = 2;
                g_ConnMgr.sendMsg(conn2, ProtocolId::MATCH_PUSH, res);
            }
        }
    }
}

MatchHandler::MatchHandler()
{
    DLOG("初始化MatchHandler模块");
    g_ProtoDisp.registerHandler(ProtocolId::MATCH_START_REQ, std::bind(&MatchHandler::onMatch, this, std::placeholders::_1, std::placeholders::_2));
    g_ProtoDisp.registerHandler(ProtocolId::MATCH_CANCEL_REQ, std::bind(&MatchHandler::onCancelMatch, this, std::placeholders::_1, std::placeholders::_2));
}

// 处理匹配请求
void MatchHandler::onMatch(const TcpConnectionPtr &conn, const Json::Value &req)
{
    DLOG("进入匹配请求处理函数");
    Json::Value res;
    uint32_t userId = req["userId"].asUInt();
    bool ret = m_matchPool.addUser(userId);
    if (ret == false)
    {
        res["success"] = false;
        res["reason"] = "开始匹配失败";
    }
    else
    {
        res["success"] = true;
        res["reason"] = "开始匹配成功";
    }
    g_ConnMgr.sendMsg(conn, ProtocolId::MATCH_START_ACK, res);
}

// 处理取消匹配请求
void MatchHandler::onCancelMatch(const TcpConnectionPtr &conn, const Json::Value &req)
{
    DLOG("进入取消匹配请求处理函数");
    Json::Value res;
    uint32_t userId = req["userId"].asUInt();
    bool ret = m_matchPool.removeUser(userId);
    if (ret == false)
    {
        res["success"] = false;
        res["reason"] = "取消匹配失败";
    }
    else
    {
        res["success"] = true;
        res["reason"] = "取消匹配成功";
    }
    g_ConnMgr.sendMsg(conn, ProtocolId::MATCH_CANCEL_ACK, res);
}