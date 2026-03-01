#pragma once

#include <jsoncpp/json/json.h>

#include "../MyMuduo/TcpServer.h"

// 账号模块协议处理入口：只做协议注册、参数解析、Service调用
class UserHandler
{
public:
    UserHandler();

    void onRegister(const TcpConnectionPtr &conn, const Json::Value &req); // 处理注册请求
    void onLogin(const TcpConnectionPtr &conn, const Json::Value &req);    // 处理登录请求

};

// 全局初始化：确保启动时注册协议
extern UserHandler g_UserHandler;