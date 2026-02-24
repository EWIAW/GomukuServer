#pragma once

#include <cstdint>
#include <vector>

#define BOARD_SIZE 15 // 棋盘大小
#define WHITE_CHESS 1 // 白方的棋子
#define BLACK_CHESS 2 // 黑方的棋子

// 房间的状态
enum GameRoomState
{
    START = 0, // 开始
    OVER = 1,  // 结束
};

class GameRoomInfo
{
public:
    GameRoomInfo(uint32_t roomId, uint32_t whiteId, uint32_t blackId);

    ~GameRoomInfo() = default;

    uint32_t getRoomCount() const { return _roomCount; }                                    // 获取房间人数
    uint32_t getRoomId() const { return _roomId; }                                          // 获取房间id
    uint32_t getWhiteId() const { return _whiteId; }                                        // 获取白棋id
    uint32_t getBlackId() const { return _blackId; }                                        // 获取黑棋id
    uint32_t getOtherId(uint32_t id) const { return id == _whiteId ? _blackId : _whiteId; } // 根据一个id获取另外一个id
    GameRoomState getState() const { return _state; }                                       // 获取房间状态

    void subCount() { _roomCount--; }                      // 减少一个人数
    void setState(GameRoomState state) { _state = state; } // 设置房间状态

    bool isValidPosition(int x, int y) const;   // 判断位置是否合法
    bool handleChess(int x, int y, int userId); // 处理下棋操作，如果出现获胜方，返回true

private:
    bool five(const int row, const int col,
              const int offset_row, const int offset_col, const int color); // 判断一个方向上是否出现五星连珠
    bool checkWin(int x, int y, int color);                                 // 判断是否出现获胜方

private:
    GameRoomState _state; // 房间状态
    uint32_t _roomCount;  // 房间人数
    uint32_t _roomId;     // 房间id
    uint32_t _whiteId;    // 白棋id
    uint32_t _blackId;    // 黑棋id

    std::vector<std::vector<int>> _board; // 棋盘
};