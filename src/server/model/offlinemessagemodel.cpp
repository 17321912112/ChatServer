#include "offlinemessagemodel.hpp"


// 添加
bool OfflineMsgModel::insert(int id, const string& msg)
{
    // 准备sql
    char sql[1024] = {0};
    sprintf(sql, "insert into OfflineMessage values(%d, '%s')", id, msg.c_str());
    // 执行sql
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            return true;
        }
    }
    return false;
}
// 查找
vector<string> OfflineMsgModel::query(int id)
{
    vector<string> msgs;
    // 准备sql
    char sql[1024] = {0};
    sprintf(sql, "select message from OfflineMessage where userid = %d", id);
    // 执行sql
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                msgs.push_back(row[0]);
            }
            mysql_free_result(res); // 释放结果集
        }
    }
    return msgs;
}
// 删除
bool OfflineMsgModel::remove(int id)
{
    // 准备sql
    char sql[1024] = {0};
    sprintf(sql, "delete from OfflineMessage where userid = %d", id);
    // 执行sql
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            return true;
        }
    }
    return false;
}