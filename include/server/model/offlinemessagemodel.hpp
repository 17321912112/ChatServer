#ifndef OFFLINEMESSAGEMODEL_H
#define OFFLINEMESSAGEMODEL_H

/**
 * OfflineMsg的数据操作类
 */
#include <vector>
#include <string>
#include "db.h"
using namespace std;
class OfflineMsgModel
{
public:
    OfflineMsgModel() = default;
    // 添加
    bool insert(int id, const string& msg);
    // 查找
    vector<string> query(int id);
    // 删除
    bool remove(int id);
};

#endif