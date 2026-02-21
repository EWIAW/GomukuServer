#pragma once

#include <jsoncpp/json/json.h>

#include "../MyMuduo/TcpServer.h"

// 游戏房间模块协议处理入口
class GameRoomHandler
{
public:
    GameRoomHandler();

    void onChat(const TcpConnectionPtr &conn, const Json::Value &req);     // 聊天处理请求
    void onChess(const TcpConnectionPtr &conn, const Json::Value &req);    // 落子处理请求
};

// 全局初始化：确保启动时注册协议
extern GameRoomHandler g_GameRoomHandler;