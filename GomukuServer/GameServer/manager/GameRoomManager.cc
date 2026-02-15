#include "GameRoomManager.h"

#include "../base/Log.h"

GameRoomManager::GameRoomManager()
    : m_roomNextId(1)
{
    DLOG("初始化GameRoomManager单例");
}

void GameRoomManager::createRoom(uint32_t userId1, uint32_t userId2)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    GameRoomInfo *room = new GameRoomInfo(m_roomNextId, userId1, userId2);
    m_userId2RoomId[userId1] = m_roomNextId;
    m_userId2RoomId[userId2] = m_roomNextId;
    m_roomId2Room[m_roomNextId] = std::shared_ptr<GameRoomInfo>(room);
    DLOG("创建房间成功, 房间ID: %d", m_roomNextId++);
}

void GameRoomManager::removeRoom(uint32_t roomId)
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
        m_userId2RoomId.erase(whiteId);
        m_userId2RoomId.erase(blackId);
        m_roomId2Room.erase(roomId);
        DLOG("销毁房间成功, 房间ID: %d", roomId);
    }
}

// 通过用户id获取房间信息
std::shared_ptr<GameRoomInfo> GameRoomManager::getRoomByUserId(uint32_t userId)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it1 = m_userId2RoomId.find(userId);
    if (it1 == m_userId2RoomId.end())
        return nullptr;

    auto it2 = m_roomId2Room.find(it1->second);
    if (it2 == m_roomId2Room.end())
        return nullptr;
    else
        return it2->second;
}

// 通过房间id获取房间信息
std::shared_ptr<GameRoomInfo> GameRoomManager::getRoomByRoomId(uint32_t roomId)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_roomId2Room.find(roomId);
    return it != m_roomId2Room.end() ? it->second : nullptr;
}