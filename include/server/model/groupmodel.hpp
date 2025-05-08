#ifndef GROUPMODEL_H
#define GROUPMODEL_H

#include "group.hpp"
#include "groupuser.hpp"
#include "db.h"
#include <vector>
using namespace std;

class GroupModel
{
public:
    // 添加群组
    bool insert(Group &group);
    // 删除群组
    bool remove(int id);
    // 添加用户到群组
    bool addToGroup(int groupid, int userid,const string& role);
    // 删除用户从群组
    bool removeFromGroup(int groupid, int userid);
    // 获取user的群组列表
    vector<Group> queryGroups(int userid);
    // 获取群组的用户列表
    vector<int> queryGroupUsers(int userid, int groupid);
private:
};


#endif