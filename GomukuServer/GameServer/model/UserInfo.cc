#include "UserInfo.h"

void UserInfo::winGame()
{
    _userPoints += 10;
    _totalCount += 1;
    _winCount += 1;
}

void UserInfo::loseGame()
{
    _userPoints -= 5;
    _totalCount += 1;
}