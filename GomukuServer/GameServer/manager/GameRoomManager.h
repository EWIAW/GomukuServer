#pragma once

#include <memory>
#include <mutex>
#include <unordered_map>

#include "../base/NonCopyable.h"
#include "../base/Singleton.h"

#include "../model/GameRoomInfo.h"

// 房间管理类：单例，线程安全
class GameRoomManager : NonCopyable
{
    friend class Singleton<GameRoomManager>;

public:
    void createRoom(uint32_t userId1, uint32_t userId2); // 创建房间
    void passiveRemoveRoom(uint32_t roomId);             // 被动销毁房间，当房间没人时触发
    void activeRemoveRoom(uint32_t roomId);              // 主动销毁房间，由程序员判断是否需要主动销毁

    void removeUser(uint32_t userId); // 将用户从房间中移除

    std::shared_ptr<GameRoomInfo> getRoomByUserId(uint32_t userId); // 通过用户id获取房间信息
    std::shared_ptr<GameRoomInfo> getRoomByRoomId(uint32_t roomId); // 通过房间id获取房间信息

private:
    GameRoomManager();

    uint32_t m_roomNextId; // 为每一个房间创建一个id
    std::mutex m_mutex;
    std::unordered_map<uint32_t, std::shared_ptr<GameRoomInfo>> m_userId2Room; // 用户id -> 房间信息
    std::unordered_map<uint32_t, std::shared_ptr<GameRoomInfo>> m_roomId2Room; // 房间id -> 房间信息
};

// 全局快捷调用
#define g_GameRoomMgr Singleton<GameRoomManager>::getInstance()