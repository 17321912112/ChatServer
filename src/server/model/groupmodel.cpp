#include "groupmodel.hpp"

bool GroupModel::insert(Group &group)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into AllGroup(groupname, groupdesc) values('%s', '%s')",
            group.getName().c_str(), group.getDesc().c_str());
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            group.setId(mysql.getConnection()->insert_id);
            return true;
        }
    }
    return false;
}
bool GroupModel::remove(int id)
{
    char sql[1024] = {0};
    sprintf(sql, "delete from AllGroup where id=%d", id);
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
bool GroupModel::addToGroup(int groupid, int userid, const string &role)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into GroupUser(groupid, userid, grouprole) values(%d, %d, '%s')",
            groupid, userid, role.c_str());
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
bool GroupModel::removeFromGroup(int groupid, int userid)
{   
    char sql[1024] = {0};
    sprintf(sql, "delete from GroupUser where groupid=%d and userid=%d", groupid, userid);
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

// 获取user的群组列表
vector<Group> GroupModel::queryGroups(int userid)
{
    vector<Group> groups;
    
    char sql[1024] = {0};
    sprintf(sql, "select a.id, a.groupname,a.groupdesc from \
            AllGroup a inner join GroupUser b on a.id=b.groupid where b.userid=%d", userid);
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                groups.emplace_back(atoi(row[0]), row[1], row[2]);
            }
            mysql_free_result(res); // 释放结果集
        }
    }

    // 查询群组的用户信息
    for (Group &group : groups)
    {
        sprintf(sql, "select a.id,a.name,a.state,b.grouprole from User a \
            inner join GroupUser b on b.userid = a.id where b.groupid=%d",
                group.getId());

        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                GroupUser user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                user.setRole(row[3]);
                group.addUser(user);
            }
            mysql_free_result(res);
        }
    }
    return groups;
}
// 获取群组的用户列表
vector<int> GroupModel::queryGroupUsers(int userid, int groupid)
{
    vector<int> userids;
    
    char sql[1024] = {0};
    sprintf(sql, "select userid from GroupUser where groupid=%d and userid!=%d", groupid, userid);
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                userids.push_back(atoi(row[0]));
            }
            mysql_free_result(res); // 释放结果集    
        }
    }
    return userids;
}