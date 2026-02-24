#include "UserDB.h"

#include "../base/Log.h"

#include "../../MysqlConnectionPool/MysqlConnectionPool.h"

// 所有MySQL语句
#define REGISTER_USER "INSERT INTO users (userName, passWord, userPoint, totalCount, winCount) VALUES ('%s', SHA2('%s', 256), 1000, 0, 0);"      // 注册新用户
#define LOGIN_USER "SELECT userId, userName, userPoint, totalCount, winCount FROM users WHERE userName = '%s' AND passWord = SHA2('%s', 256);"   // 登录验证
#define WIN_USER "UPDATE users SET userPoint = userPoint + 10, totalCount = totalCount + 1, winCount = winCount + 1 WHERE userId = %d;"          // 玩家获胜
#define LOST_USER "UPDATE users SET userPoint = GREATEST(userPoint - 5, 0), totalCount = totalCount + 1, winCount = winCount WHERE userId = %d;" // 玩家失败

UserDB::UserDB()
{
    DLOG("初始化UserDB单例");
}

// 注册玩家
std::pair<bool, std::string> UserDB::registerUser(const std::string username, const std::string password)
{
    // 获取数据库连接
    MysqlConnectionPool *mcp = MysqlConnectionPool::getConnectionPool();
    std::shared_ptr<MysqlConnection> con = mcp->getMysqlConnection();

    // 尝试插入新用户
    char sql[4096] = {0}; // 要执行的sql语句
    sprintf(sql, REGISTER_USER, username.c_str(), password.c_str());
    std::pair<bool, std::string> ret = con->update(sql);
    if (ret.first == false)
    {
        return std::make_pair(false, "该用户名已经存在");
    }
    else
    {
        return std::make_pair(true, "注册成功");
    }
}

// 登录验证
std::pair<std::pair<bool, std::string>, std::vector<std::string>> UserDB::loginUser(const std::string username, const std::string password)
{
    // 获取数据库连接
    MysqlConnectionPool *mcp = MysqlConnectionPool::getConnectionPool();
    std::shared_ptr<MysqlConnection> con = mcp->getMysqlConnection();

    // 进行登录
    char sql[4096] = {0};
    sprintf(sql, LOGIN_USER, username.c_str(), password.c_str());
    MYSQL_RES *res = con->query(sql);

    // 判断结果集数据是否为1
    int row_num = mysql_num_rows(res);
    if (row_num == 0)
    {
        return std::make_pair(std::make_pair(false, "用户名或密码错误"), std::vector<std::string>());
    }
    else
    {
        MYSQL_ROW row = mysql_fetch_row(res);
        std::vector<std::string> ret;
        for (int i = 0; i < mysql_num_fields(res); i++)
        {
            ret.push_back(row[i]);
        }
        return std::make_pair(std::make_pair(true, "登录成功"), ret);
    }
}

// 游戏获胜，添加相应积分 和 添加对战场数
void UserDB::winGame(uint32_t userId)
{
    // 获取数据库连接
    MysqlConnectionPool *mcp = MysqlConnectionPool::getConnectionPool();
    std::shared_ptr<MysqlConnection> con = mcp->getMysqlConnection();

    // 修改对应信息
    char sql[4096] = {0};
    sprintf(sql, WIN_USER, userId);
    MYSQL_RES *res = con->query(sql);
}

// 游戏失败，减少相应积分 和 添加对战场数
void UserDB::loseGame(uint32_t userId)
{
    // 获取数据库连接
    MysqlConnectionPool *mcp = MysqlConnectionPool::getConnectionPool();
    std::shared_ptr<MysqlConnection> con = mcp->getMysqlConnection();

    // 修改对应信息
    char sql[4096] = {0};
    sprintf(sql, LOST_USER, userId);
    MYSQL_RES *res = con->query(sql);
}