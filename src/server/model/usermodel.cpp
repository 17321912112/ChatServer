#include "usermodel.hpp"

#include "db.h"
#include <iostream>
#include <muduo/base/Logging.h>
using namespace std;

UserModel::UserModel()
{

}
// 添加用户
bool UserModel::insert(User& user)
{
    MySQL mysql;
    if (mysql.connect())
    {
        char sql[1024] = {0};
        sprintf(sql, "insert into User(name,password) values('%s','%s')",
            user.getName().c_str(), user.getPassword().c_str());
        LOG_INFO << "sql:" << sql;
        if (mysql.update(sql)) 
        {
            user.setId(mysql.getConnection()->insert_id);
            return true;
        } 
    }
    return false;
}

User UserModel::query(int id)
{
    MySQL mysql;
    User user;
    if (mysql.connect())
    {
        char sql[1024] = {0};
        sprintf(sql, "select name,password,state from User where id = %d", id);
        LOG_INFO << "sql:" << sql;
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            auto row = mysql_fetch_row(res);
            if (row != nullptr)
            {
                LOG_INFO << "queryed row[0]:" << row[0];
                user.setName(row[0]);
                user.setPassword(row[1]);
                user.setState(row[2]);
                user.setId(id);
                mysql_free_result(res); // 释放结果集
            }
        }
    }
    if (user.getId() == -1) 
    {
        LOG_INFO << "query user failed!";
    }
    else 
    {
        LOG_INFO << "query user success!";
    }
    return user;
}

bool UserModel::updateState(const User& user)
{
    MySQL mysql;
    if (mysql.connect()) 
    {
        char sql[1024] = {0};
        sprintf(sql, "update User set state='%s' where id=%d", user.getState().c_str(), user.getId());
        LOG_INFO << "updateState sql:" << sql;
        if (mysql.update(sql)) 
        {
            return true;
        }
    }
    return false;
}

void UserModel::resetState()
{
    MySQL mysql;
    if (mysql.connect()) 
    {
        char sql[1024] = {0};
        sprintf(sql, "update User set state='offline' where state='online'");
        LOG_INFO << "updateState sql:" << sql;
        mysql.update(sql);
    }
}