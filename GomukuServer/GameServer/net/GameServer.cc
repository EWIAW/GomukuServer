#include "GameServer.h"

#include <jsoncpp/json/json.h>

#include "../base/Log.h"

#include "../manager/ConnManager.h"

#include "ProtocolDispatcher.h"

GameServer::GameServer(EventLoop *loop, const InetAddress &addr, const std::string &name)
    : _server(loop, addr, name),
      _loop(loop)
{
    // 注册回调函数
    _server.setConnectionCallback(std::bind(&GameServer::onConnection, this, std::placeholders::_1));
    _server.setMessageCallback(std::bind(&GameServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    // 设置合适的loop线程数量 loopthread
    _server.setThreadNum(4);

    // server_.setEnableInactiveRelease(10);
}

// 服务开始运行
void GameServer::start()
{
    _server.start();
}

// 连接建立或者断开的回调
void GameServer::onConnection(const TcpConnectionPtr &conn)
{
    if (conn->connected())
    {
        LOG_DEBUG("Connection UP : %s", conn->peerAddress().GetIpPort().c_str());
        g_ConnMgr.addConn(conn);
    }
    else
    {
        LOG_DEBUG("Connection DOWN : %s", conn->peerAddress().GetIpPort().c_str());
        g_ConnMgr.removeConn(conn);
    }
}

void GameServer::onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
{
    g_ProtoDisp.dispatch(conn, buf);
}