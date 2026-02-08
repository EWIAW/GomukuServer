#pragma once

#include "../MyMuduo/TcpServer.h"

class GameServer
{
private:
    EventLoop *_loop;
    TcpServer _server;

public:
    GameServer(EventLoop *loop, const InetAddress &addr, const std::string &name);
    void start();

private:
    void onConnection(const TcpConnectionPtr &conn);                           // 连接建立或者断开的回调
    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time); // 可读写事件回调
};