#include "UserHandler.h"

#include "../db/UserDB.h"

#include "../base/Log.h"
#include "../base/ProtocolId.h"

#include "../manager/ConnManager.h"
#include "../manager/UserManager.h"

#include "../model/UserInfo.h"

#include "../net/ProtocolDispatcher.h"

// 全局账号处理器：程序启动时自动初始化，注册协议处理函数
UserHandler g_UserHandler;

UserHandler::UserHandler()
{
    // 注册账号模块的协议处理函数
    DLOG("初始化UserHandler模块");
    g_ProtoDisp.registerHandler(ProtocolId::REGISTER_REQ, std::bind(&UserHandler::onRegister, this, std::placeholders::_1, std::placeholders::_2));
    g_ProtoDisp.registerHandler(ProtocolId::LOGIN_REQ, std::bind(&UserHandler::onLogin, this, std::placeholders::_1, std::placeholders::_2));
}

// 处理注册请求（包体格式：account|password|name(可选)）
void UserHandler::onRegister(const TcpConnectionPtr &conn, const Json::Value &req)
{
    DLOG("进入注册请求处理函数");
    Json::Value res;
    std::string username = req["username"].asCString();
    std::string password = req["password"].asCString();

    std::pair<bool, std::string> ret = g_UserDB.registerUser(username, password);
    res["success"] = ret.first;
    res["reason"] = ret.second;
    g_ConnMgr.sendMsg(conn, ProtocolId::REGISTER_ACK, res);
}

// 处理登录请求（包体格式：account|password）
void UserHandler::onLogin(const TcpConnectionPtr &conn, const Json::Value &req)
{
    DLOG("进入登录请求处理函数");
    Json::Value res;
    std::string username = req["username"].asCString();
    std::string password = req["password"].asCString();

    std::pair<std::pair<bool, std::string>, std::vector<std::string>> ret = g_UserDB.loginUser(username, password);
    if (ret.first.first == false)
    {
        res["success"] = false;
        res["reason"] = ret.first.second;
    }
    else
    {
        // 登录成功，将用户信息保存到用户管理器中
        uint32_t userId = std::stoi(ret.second[0]);
        std::string userName = ret.second[1];
        uint32_t userPoint = std::stoi(ret.second[2]);
        uint32_t totalCount = std::stoi(ret.second[3]);
        uint32_t winCount = std::stoi(ret.second[4]);

        UserInfo *user = new UserInfo(userName, userId, userPoint, totalCount, winCount, conn);
        g_UserMgr.addOnlineUser(user);

        res["success"] = true;
        res["reason"] = ret.first.second;
        res["userId"] = userId;
        res["userName"] = userName;
        res["userPoint"] = userPoint;
        res["totalCount"] = totalCount;
        res["winCount"] = winCount;
    }
    g_ConnMgr.sendMsg(conn, ProtocolId::LOGIN_ACK, res);
}