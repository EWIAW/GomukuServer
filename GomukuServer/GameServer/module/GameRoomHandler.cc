#include "GameRoomHandler.h"

#include "../base/Log.h"
#include "../base/ProtocolId.h"

#include "../db/UserDB.h"

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
    g_ProtoDisp.registerHandler(ProtocolId::EXIT_ROOM_REQ, std::bind(&GameRoomHandler::onExitRoom, this, std::placeholders::_1, std::placeholders::_2));
}

// 聊天处理请求
void GameRoomHandler::onChat(const TcpConnectionPtr &conn, const Json::Value &req)
{
    DLOG("进入游戏房间聊天处理函数");
    Json::Value res;
    std::string message = req["message"].asCString();
    res["message"] = message;

    uint32_t sendId = g_UserMgr.getUserIdByConn(conn); // 发送方ID
    uint32_t recvId = g_GameRoomMgr.getUserIdFromOtherId(sendId);
    TcpConnectionPtr sendConn = conn;
    TcpConnectionPtr recvConn = g_UserMgr.getConnByUserId(recvId);

    g_ConnMgr.sendMsg(recvConn, ProtocolId::GAME_CHAT_ACK, res);
}

// 下棋处理请求
void GameRoomHandler::onChess(const TcpConnectionPtr &conn, const Json::Value &req)
{
    DLOG("进入落子处理函数");
    Json::Value res;
    Json::Value winres;
    Json::Value loseres;
    int x = req["x"].asInt();
    int y = req["y"].asInt();

    uint32_t sendId = g_UserMgr.getUserIdByConn(conn); // 发送方ID
    uint32_t recvId = g_GameRoomMgr.getUserIdFromOtherId(sendId);
    TcpConnectionPtr sendConn = conn;
    TcpConnectionPtr recvConn = g_UserMgr.getConnByUserId(recvId);

    bool ret = g_GameRoomMgr.handChess(x, y, sendId);

    res["x"] = x;
    res["y"] = y;

    if (ret == true)//如果出现获胜方
    {
        winres["x"] = x;
        winres["y"] = y;
        winres["success"] = true;
        winres["win"] = true;

        loseres["x"] = x;
        loseres["y"] = y;
        loseres["success"] = true;
        loseres["win"] = false;

        g_UserDB.winGame(sendId);
        g_UserDB.loseGame(recvId);

        g_UserMgr.winGame(sendId);
        g_UserMgr.loseGame(recvId);

        //获取获胜用户的积分、总对局数、获胜场数
        uint32_t winUserPoints = g_UserMgr.getUserPoints(sendId);
        uint32_t winUserTotalCount = g_UserMgr.getUserTotalCount(sendId);
        uint32_t winUserWinCount = g_UserMgr.getUserWinCount(sendId);

        winres["userPoint"] = winUserPoints;
        winres["totalCount"] = winUserTotalCount;
        winres["winCount"] = winUserWinCount;
        g_ConnMgr.sendMsg(sendConn, ProtocolId::CHESS_DOWN_ACK, winres);

        //获取失败用户的积分、总对局数、获胜场数
        uint32_t loseUserPoints = g_UserMgr.getUserPoints(recvId);
        uint32_t loseUserTotalCount = g_UserMgr.getUserTotalCount(recvId);
        uint32_t loseUserWinCount = g_UserMgr.getUserWinCount(recvId);

        loseres["userPoint"] = loseUserPoints;
        loseres["totalCount"] = loseUserTotalCount;
        loseres["winCount"] = loseUserWinCount;
        g_ConnMgr.sendMsg(recvConn, ProtocolId::CHESS_DOWN_ACK, loseres);
    }
    else//如果没有出现获胜方
    {
        res["success"] = false;
        g_ConnMgr.sendMsg(recvConn, ProtocolId::CHESS_DOWN_ACK, res);
    }
}

// 退出房间请求
void GameRoomHandler::onExitRoom(const TcpConnectionPtr &conn, const Json::Value &req)
{
    DLOG("进入退出房间处理函数");
    Json::Value res;

    uint32_t userId = g_UserMgr.getUserIdByConn(conn);
    g_GameRoomMgr.removeUser(userId);
}
