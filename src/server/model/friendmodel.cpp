#include "friendmodel.hpp"
#include "db.h"

// 添加好友关系
bool FriendModel::insert(int userid, int friendid)
{
    MySQL mysql;
    
    char sql[1024] = {0};
    sprintf(sql, "INSERT INTO Friend (userid, friendid) VALUES (%d, %d)", userid, friendid);
    if (mysql.connect())
    {
        return mysql.update(sql);
    }
    return false;
}
// 返回用户好友列表
vector<User> FriendModel::getFriends(int userid)
{
    MySQL mysql;
    char sql[1024] = {0};
    sprintf(sql, "SELECT u.id,u.name,u.state FROM Friend f inner join User u on u.id=f.friendid WHERE f.userid=%d", userid);
    vector<User> friends;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                friends.push_back(user);
            }
            mysql_free_result(res); // 释放结果集
        }
    }
    return friends;
}