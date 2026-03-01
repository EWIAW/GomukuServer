#pragma once
#include <string>

#include "../MyMuduo/TcpServer.h"
#include "../base/NonCopyable.h"

// 玩家的状态
enum UserState
{
    OFFLINE = 0,  // 离线
    ONLINE = 1,   // 在线（空闲）
    MATCHING = 2, // 匹配中
    GAMING = 3    // 对战中
};

// 用户信息
class UserInfo : NonCopyable
{
public:
    UserInfo(std::string userName, uint32_t userId, uint32_t userPoints,
             uint32_t totalCount, uint32_t winCount, TcpConnectionPtr con)
        : _state(ONLINE),
          _userName(userName),
          _userId(userId),
          _userPoints(userPoints),
          _totalCount(totalCount),
          _winCount(winCount),
          _con(con)
    {
    }

    ~UserInfo() = default;

    UserState getState() const { return _state; }
    std::string getUserName() const { return _userName; }
    uint32_t getUserId() const { return _userId; }
    uint32_t getUserPoints() const { return _userPoints; }
    uint32_t getTotalCount() const { return _totalCount; }
    uint32_t getWinCount() const { return _winCount; }
    TcpConnectionPtr getConn() const { return _con; }

    void setState(UserState state) { _state = state; }

    void winGame();
    void loseGame();

private:
    UserState _state;      // 玩家所处的状态
    std::string _userName; // 用户名
    uint32_t _userId;      // 用户id
    uint32_t _userPoints;  // 用户积分
    uint32_t _totalCount;  // 用户总对战场数
    uint32_t _winCount;    // 用户获胜场数

    TcpConnectionPtr _con; // 用户的连接
};
