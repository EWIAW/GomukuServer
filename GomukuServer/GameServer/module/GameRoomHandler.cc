#include "GameRoomHandler.h"

#include "../base/Log.h"
#include "../base/ProtocolId.h"

#include "../manager/ConnManager.h"
#include "../manager/GameRoomManager.h"
#include "../manager/UserManager.h"

#include "../net/ProtocolDispatcher.h"

// 全局账号处理器：程序启动时自动初始化，注册协议处理函数
GameRoomHandler g_GameRoomHandler;

GameRoomHandler::GameRoomHandler()
{
    // 注册游戏房间模块的协议处理函数
    DLOG("初始化GameRoomHandler模块");
    g_ProtoDisp.registerHandler(ProtocolId::GAME_CHAT_REQ, std::bind(&GameRoomHandler::onChat, this, std::placeholders::_1, std::placeholders::_2));
    g_ProtoDisp.registerHandler(ProtocolId::CHESS_DOWN_REQ, std::bind(&GameRoomHandler::onChess, this, std::placeholders::_1, std::placeholders::_2));
}

// 聊天处理请求
void GameRoomHandler::onChat(const TcpConnectionPtr &conn, const Json::Value &req)
{
    DLOG("进入游戏房间聊天处理函数");
    Json::Value res;
    std::string message = req["message"].asCString();
    res["message"] = message;

    uint32_t sendId = g_UserMgr.getUserIdByConn(conn); // 发送方ID
    uint32_t recvId = g_GameRoomMgr.getRoomByUserId(sendId)->getOtherId(sendId);
    TcpConnectionPtr sendConn = conn;
    TcpConnectionPtr recvConn = g_UserMgr.getConnByUserId(recvId);

    g_ConnMgr.sendMsg(recvConn, ProtocolId::GAME_CHAT_ACK, res);
}

// 下棋处理请求
void GameRoomHandler::onChess(const TcpConnectionPtr &conn, const Json::Value &req)
{
    DLOG("进入落子处理函数");
    Json::Value res;
    int x = req["x"].asInt();
    int y = req["y"].asInt();

    uint32_t sendId = g_UserMgr.getUserIdByConn(conn); // 发送方ID
    uint32_t recvId = g_GameRoomMgr.getRoomByUserId(sendId)->getOtherId(sendId);
    TcpConnectionPtr sendConn = conn;
    TcpConnectionPtr recvConn = g_UserMgr.getConnByUserId(recvId);

    std::shared_ptr<GameRoomInfo> room = g_GameRoomMgr.getRoomByUserId(sendId);
    bool ret = room->handleChess(x, y, sendId);

    res["x"] = x;
    res["y"] = y;
    if (ret == true)
    {
        res["success"] = true;
        res["win"] = true;
        g_ConnMgr.sendMsg(sendConn, ProtocolId::CHESS_DOWN_ACK, res);
        res["win"] = false;
        g_ConnMgr.sendMsg(recvConn, ProtocolId::CHESS_DOWN_ACK, res);
    }
    else
    {
        res["success"] = false;
        g_ConnMgr.sendMsg(recvConn, ProtocolId::CHESS_DOWN_ACK, res);
    }
}