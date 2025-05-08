#ifndef USERMODEL_H
#define USERMODEL_H

#include "user.hpp"


/**
 * User的数据操作类
 */

class UserModel
{
public:
    UserModel();
    // 添加用户
    bool insert(User& user);
    // 查找用户
    User query(int id);
    // 更新用户
    bool updateState(const User& user);
    // 重置
    void resetState();
private:
};


#endif