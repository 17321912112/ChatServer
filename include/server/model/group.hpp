#ifndef GROUP_H
#define GROUP_H

#include <string>
#include <vector>
#include "groupuser.hpp"
using namespace std;
class Group
{
public:
    Group(int id = -1, string name = "", string desc = "")
        : id(id), name(name), desc(desc) {}
    void setId(int id) { this->id = id; }
    void setName(string name) { this->name = name; }
    void setDesc(string desc) { this->desc = desc; }
    int getId() const { return id; }
    string getName() const { return name; }
    string getDesc() const { return desc; }

    void addUser(const GroupUser &user) { users.push_back(user); }
    vector<GroupUser> getUsers() const { return users; }
private:
    int id;
    string name;
    string desc;
    vector<GroupUser> users;
};

#endif