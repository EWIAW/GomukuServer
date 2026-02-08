#pragma once

#include <unordered_map>
#include <functional>

#include <jsoncpp/json/json.h>

#include "../MyMuduo/TcpServer.h"

#include "../base/NonCopyable.h"
#include "../base/ProtocolId.h"
#include "../base/Singleton.h"

// 协议处理函数类型：所有业务模块的处理函数都遵循此格式
using ProtocolHandler = std::function<void(const TcpConnectionPtr &, const Json::Value &)>;

// 协议分发器：单例，统一分发所有协议，是网络层与业务层的核心桥梁
class ProtocolDispatcher : NonCopyable
{
    friend class Singleton<ProtocolDispatcher>;

public:
    void registerHandler(const ProtocolId protoId, ProtocolHandler handler); // 注册协议处理函数：协议号 → 处理函数
    void dispatch(const TcpConnectionPtr &conn, Buffer *buf);                // 分发协议：解析数据→找到处理函数→执行（muduo的onMessage回调调用此方法）

private:
    ProtocolDispatcher();

    std::unordered_map<int, ProtocolHandler> m_handlerMap; // 协议号 → 处理函数
};

// 全局快捷调用
#define g_ProtoDisp Singleton<ProtocolDispatcher>::getInstance()