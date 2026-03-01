#include "GameRoomManager.h"

#include "../base/Log.h"

#include "../manager/UserManager.h"

GameRoomManager::GameRoomManager()
    : m_roomNextId(1)
{
    DLOG("初始化GameRoomManager单例");
}

void GameRoomManager::createRoom(uint32_t userId1, uint32_t userId2)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto sp = std::make_shared<GameRoomInfo>(m_roomNextId, userId1, userId2);
    m_userId2Room[userId1] = sp;
    m_userId2Room[userId2] = sp;
    m_roomId2Room[m_roomNextId] = sp;
    DLOG("创建房间成功, 房间ID: %d", m_roomNextId++);
}

// 被动销毁房间，当房间没人时触发
void GameRoomManager::passiveRemoveRoom(uint32_t roomId)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    m_roomId2Room.erase(roomId);
    DLOG("被动销毁房间成功, 房间ID: %d", roomId);
}

// 主动销毁房间，由程序员判断是否需要主动销毁
void GameRoomManager::activeRemoveRoom(uint32_t roomId)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    // 先获取该房间的两个用户
    auto it = m_roomId2Room.find(roomId);
    if (it == m_roomId2Room.end())
    {
        return;
    }
    else
    {
        std::shared_ptr<GameRoomInfo> gameRoom = it->second;
        uint32_t whiteId = gameRoom->getWhiteId();
        uint32_t blackId = gameRoom->getBlackId();
        m_userId2Room.erase(whiteId);
        m_userId2Room.erase(blackId);
        m_roomId2Room.erase(roomId);
        DLOG("主动销毁房间成功, 房间ID: %d", roomId);
    }
}

// 将用户从房间中移除
void GameRoomManager::removeUser(uint32_t userId)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_userId2Room.find(userId);
    if (it == m_userId2Room.end())
        return;

    std::shared_ptr<GameRoomInfo> room = it->second;
    uint32_t roomId = room->getRoomId();

    // 移除用户并判断是否需要销毁房间
    m_userId2Room.erase(userId);
    room->subCount();
    if (room->getRoomCount() == 0)
    {
        passiveRemoveRoom(roomId);
    }

    // 更新用户状态
    g_UserMgr.setUserState(userId, UserState::ONLINE);
}

// 通过一个用户的id，找到该用户房间内的另一个用户的id
uint32_t GameRoomManager::getUserIdFromOtherId(uint32_t id)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_userId2Room.find(id);
    if (it == m_userId2Room.end())
        return 0;

    std::shared_ptr<GameRoomInfo> room = it->second;
    return room->getOtherId(id);
}

// 处理落子操作，如果出现获胜方，返回true
bool GameRoomManager::handChess(int x, int y, uint32_t sendId)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_userId2Room.find(sendId);
    if (it == m_userId2Room.end())
        return false;

    std::shared_ptr<GameRoomInfo> room = it->second;
    return room->handleChess(x, y, sendId);
}

// 通过用户id获取房间信息
std::shared_ptr<GameRoomInfo> GameRoomManager::getRoomByUserId(uint32_t userId)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_userId2Room.find(userId);
    return it != m_userId2Room.end() ? it->second : nullptr;
}

// 通过房间id获取房间信息
std::shared_ptr<GameRoomInfo> GameRoomManager::getRoomByRoomId(uint32_t roomId)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_roomId2Room.find(roomId);
    return it != m_roomId2Room.end() ? it->second : nullptr;
}