#include "ConnManager.h"

#include "../base/Log.h"

#include "../manager/UserManager.h"

ConnManager::ConnManager()
{
    DLOG("初始化ConnnManager单例");
}

// 添加连接
void ConnManager::addConn(const TcpConnectionPtr &conn)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_conns.insert(conn);
    DLOG("建立连接 : %s", conn->peerAddress().GetIpPort().c_str());
}

// 移除连接（主动关闭/断连）
void ConnManager::removeConn(const TcpConnectionPtr &conn)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_conns.erase(conn);

    // 断连清理：移除在线玩家，更新状态
    uint32_t userId = g_UserMgr.getUserIdByConn(conn);
    if (userId != 0)
    {
        g_UserMgr.removeOnlineUser(userId);
        // 后续扩展：匹配池移出、对局判负等
    }

    DLOG("断开连接 : %s", conn->peerAddress().GetIpPort().c_str());
}

// 检查连接是否有效
bool ConnManager::isConnValid(const TcpConnectionPtr &conn)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_conns.count(conn) > 0;
}

// 向指定连接发送消息
void ConnManager::sendMsg(const TcpConnectionPtr &conn, const int &protocol, const Json::Value &msg)
{
    if (!isConnValid(conn))
        return;

    // 构造需要发送的json消息
    Json::Value response;
    response["protocol"] = protocol;
    response["data"] = msg;

    // 将json转换成字符串
    Json::FastWriter writer;
    std::string jsonStr = writer.write(response);

    // 添加消息头
    int size = jsonStr.size();
    int nsize = htonl(size); // 网络字节序长度

    // 真正发送的json消息
    std::string realmsg;
    realmsg.resize(4 + size);
    memcpy(&realmsg[0], &nsize, sizeof(nsize));
    memcpy(&realmsg[4], jsonStr.c_str(), size);

    DLOG("发送消息大小 : %d", size);
    DLOG(response.toStyledString().c_str());
    conn->send(realmsg);
}

// 向指定玩家发送消息
// void sendMsgToUser(UserId userId, const Json::Value &msg)
// {
//     if (!isConnValid(conn))
//         return;
//     User *user = g_UserMgr.getUserById(userId);
//     if (user)
//     {
//         sendMsg(user->getConn(), msg);
//     }
// }