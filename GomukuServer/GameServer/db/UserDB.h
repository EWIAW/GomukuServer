#pragma once

#include "../base/NonCopyable.h"
#include "../base/Singleton.h"

#include <vector>
#include <string>

// 账号数据库操作：所有账号相关DB操作都在此类，业务模块不直接操作DB
class UserDB : NonCopyable
{
    friend class Singleton<UserDB>;

public:
    std::pair<bool, std::string> registerUser(const std::string username, const std::string password);                                   // 注册玩家
    std::pair<std::pair<bool, std::string>, std::vector<std::string>> loginUser(const std::string username, const std::string password); // 登录验证

private:
    UserDB();
};

// 全局快捷调用
#define g_UserDB Singleton<UserDB>::getInstance()   