#include "GameRoomInfo.h"

GameRoomInfo::GameRoomInfo(uint32_t roomId, uint32_t whiteId, uint32_t blackId)
    : _state(START),
      _roomId(roomId),
      _whiteId(whiteId),
      _blackId(blackId)
{
    // 初始化棋盘
    _board.resize(BOARD_SIZE);
    for (auto &vec : _board)
    {
        vec.resize(BOARD_SIZE);
        std::fill(vec.begin(), vec.end(), 0);
    }
}

// 判断位置是否合法
bool GameRoomInfo::isValidPosition(int x, int y) const
{
    if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE)
        return false;
    if (_board[x][y] != 0)
        return false;
    return true;
}

// 处理下棋操作
bool GameRoomInfo::handleChess(int x, int y, int userId)
{
    int chess = 0;
    if (userId == _whiteId)
        chess = WHITE_CHESS;
    else
        chess = BLACK_CHESS;

    _board[x][y] = chess;
    return checkWin(x, y, chess);
}

// 判断一个方向上是否出现五星连珠
bool GameRoomInfo::five(const int row, const int col,
                        const int offset_row, const int offset_col, const int color)
{
    int count = 1;
    int tmp_row = row;
    int tmp_col = col;

    tmp_row += offset_row;
    tmp_col += offset_col;
    while (tmp_row >= 0 && tmp_row < BOARD_SIZE && tmp_col >= 0 && tmp_col < BOARD_SIZE)
    {
        if (_board[tmp_row][tmp_col] == color)
        {
            count++;
            tmp_row += offset_row;
            tmp_col += offset_col;
        }
        else
        {
            break;
        }
    }
    tmp_row = row;
    tmp_col = col;
    tmp_row -= offset_row;
    tmp_col -= offset_col;
    while (tmp_row >= 0 && tmp_row < BOARD_SIZE && tmp_col >= 0 && tmp_col < BOARD_SIZE)
    {
        if (_board[tmp_row][tmp_col] == color)
        {
            count++;
            tmp_row -= offset_row;
            tmp_col -= offset_col;
        }
        else
        {
            break;
        }
    }
    if (count == 5)
        return true;
    else
        return false;
}

// 判断是否出现获胜方
bool GameRoomInfo::checkWin(int x, int y, int color)
{
    if (five(x, y, 1, 0, color) ||
        five(x, y, 0, 1, color) ||
        five(x, y, -1, -1, color) ||
        five(x, y, -1, 1, color))
    {
        return true;
    }
    return false;
}
