#include "ProtocolDispatcher.h"

#include "../base/Log.h"

ProtocolDispatcher::ProtocolDispatcher()
{
    DLOG("初始化ProtocolDispatcher单例");
}

// 注册协议处理函数：协议号 → 处理函数
void ProtocolDispatcher::registerHandler(ProtocolId protoId, ProtocolHandler handler)
{
    if (!handler)
    {
        ELOG("注册处理函数失败 : ProtocolId = %u, handler is null", static_cast<uint32_t>(protoId));
        return;
    }
    m_handlerMap[protoId] = std::move(handler);
    DLOG("注册处理函数成功 : protoId = %u", static_cast<uint32_t>(protoId));
}

// 分发协议：解析数据→找到处理函数→执行（muduo的onMessage回调调用此方法）
void ProtocolDispatcher::dispatch(const TcpConnectionPtr &conn, Buffer *buf)
{
    // 先判断是否有一条完整的消息
    int size = 0;                 // 消息的长度
    if (buf->readableBytes() > 4) // 说明可以获取到一条消息的消息头
    {
        memcpy(&size, buf->peek(), sizeof(size)); // 先获取消息头
        size = ntohl(size);                       // 转换字节序

        // 判断是否有恶意数据，后期可以根据消息大小进行调整
        if (size > 1024)
        {
            WLOG("接收的数据大小超过限定值 : size = %d , 限定大小 : maxSize = %d", size, 1024);
            return;
        }

        if (size >= buf->readableBytes() - 4) // 说明有一条完整的消息
        {
            buf->retrieve(sizeof(size)); // 去除长度消息头
            std::string msg = buf->retrieveAsString(size);

            // 解析JSON消息
            Json::Value root;
            Json::Reader reader;
            if (!reader.parse(msg, root))
            {
                WLOG("解析JSON消息失败 : %s", msg.c_str());
                return;
            }

            int msgProtocol = root["protocol"].asInt(); // 获取协议号
            Json::Value data = root["data"];            // 获取真正的数据
            auto handlerIt = m_handlerMap.find(msgProtocol);
            if (handlerIt != m_handlerMap.end())
            {
                DLOG("接收消息的大小 : size = %d", size);
                DLOG(root.toStyledString().c_str());
                handlerIt->second(conn, data);
            }
            else
            {
                WLOG("未知消息类型，其协议号 : %d", msgProtocol);
            }
        }
    }
}