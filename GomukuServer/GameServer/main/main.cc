#include "../net/GameServer.h"

#include "../base/Log.h"

#include "../db/UserDB.h"

#include "../manager/ConnManager.h"
#include "../manager/UserManager.h"

#include "../net/ProtocolDispatcher.h"

//初始化所有单例
void initAllSingleton()
{
    DLOG("initAllSingleton");
    DLOG("初始化db文件夹");
    g_UserDB;
    DLOG("初始化manager文件夹");
    g_ConnMgr;
    g_UserMgr;
    DLOG("初始化net文件夹");
}

int main()
{
    initAllSingleton();

    EventLoop loop;
    InetAddress addr(3489, "0.0.0.0");
    GameServer server(&loop, addr, "GobandServer"); // Acceptor non-blocking listenfd  create bind
    server.start();                                 // listen  loopthread  listenfd => acceptChannel => mainLoop =>
    loop.loop();                                    // 启动mainLoop的底层Poller

    return 0;
}