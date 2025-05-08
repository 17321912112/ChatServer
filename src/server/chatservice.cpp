#include "chatservice.hpp"
#include "public.hpp"
#include "redis.hpp"
#include <muduo/base/Logging.h>
#include <mutex>

ChatService* ChatService::getInstance()
{
    static ChatService instance;
    return &instance;
}
// 获取消息对应的处理器
MsgHandler ChatService::getMsgHandler(int msgid)
{
    if (_msgHandlerMap.find(msgid) != _msgHandlerMap.end())
    {
        return _msgHandlerMap[msgid];
    }
    else
    {
        return [msgid](const TcpConnectionPtr& , json& , Timestamp){
            LOG_ERROR << "msgid:" << msgid << "handler is not found!";
        };
    }
}
ChatService::ChatService() 
    :_msgHandlerMap()
    ,_userModel()
    ,_redis()
{
    // 注册消息处理回调
    _msgHandlerMap.insert({ LOGIN_MSG, std::bind(&ChatService::login, this, ::_1, ::_2, ::_3) });
    _msgHandlerMap.insert({ REG_MSG, std::bind(&ChatService::reg, this, ::_1, ::_2, ::_3) });
    _msgHandlerMap.insert({ ONE_CHAT_MSG, std::bind(&ChatService::oneToOneChat, this, ::_1, ::_2, ::_3) });
    _msgHandlerMap.insert({ ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, ::_1, ::_2, ::_3) });
    _msgHandlerMap.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, ::_1, ::_2, ::_3) });
    _msgHandlerMap.insert({ADD_GROUP_MSG, std::bind(&ChatService::joinGroup, this, ::_1, ::_2, ::_3) });
    _msgHandlerMap.insert({ GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, ::_1, ::_2, ::_3) });

    if (_redis.connect())
    {
        _redis.init_notify_handler(std::bind(&ChatService::handleRedisSubscribeMessage, this, ::_1, ::_2));
    }
}
// 处理登录业务
void ChatService::login(const TcpConnectionPtr& conn, json& js, Timestamp time) 
{
    LOG_INFO << "do login...";
    int id = js["id"];
    string password = js["password"];
    User user = _userModel.query(id);
    json response;
    response["msgid"] = LOGIN_MSG_ACK;
    if (user.online())
    {
        response["errno"] = 1;
        response["errmsg"] = "用户已登录,不允许重复登陆!";
        conn->send(response.dump());
    }
    else if(user.getId() == id && user.getPassword() == password) 
    {   
        response["errno"] = 0;
        response["id"] = id;
        response["name"] = user.getName();
        // 登录成功,修改用户状态
        user.setState("online");
        _userModel.updateState(user);
        // 用户登陆后,订阅该用户的消息
        _redis.subscribe(id);
        // 读离线消息
        vector<string> offlineMsgs = _offlineMsgModel.query(id);
        if (!offlineMsgs.empty())
        {
            response["offlinemsg"] = offlineMsgs;
            // 读取该用户的离线消息后，把该用户的所有离线消息删除掉
            _offlineMsgModel.remove(id);
        }

        // 显示好友列表
        vector<User> friends = _friendModel.getFriends(id);
        if (!friends.empty())
        {
            vector<string> friendList;
            for (auto& user : friends)
            {
                json js;
                js["id"] = user.getId();
                js["name"] = user.getName();
                js["state"] = user.getState();
                friendList.push_back(js.dump());
            }
            response["friends"] = friendList;
        }
        // 显示群组列表
        vector<Group> groups = _groupModel.queryGroups(id);
        vector<string> groupList;
        if (!groups.empty())
        {
            for (auto& group : groups)
            {
                json js;
                js["id"] = group.getId();
                js["groupname"] = group.getName();
                js["groupdesc"] = group.getDesc();
                vector<string> users;
                vector<GroupUser> usersInGroup = group.getUsers();
                for (auto& u : usersInGroup)
                {
                    json j;
                    j["id"] = u.getId();
                    j["name"] = u.getName();
                    j["state"] = u.getState();
                    j["role"] = u.getRole();
                    users.emplace_back(j.dump());
                }
                js["users"] = users;
                groupList.push_back(js.dump());
            }
            response["groups"] = groupList;
        }
        // 发送登录成功消息到客户端
        LOG_INFO << "send:" << response.dump();
        conn->send(response.dump());
        // 记录用户的连接
        {
            std::lock_guard<std::mutex> lock(_connMutex);
            _userConnMap.insert({ id, conn });
        }
    }
    else
    {
        response["errno"] = 1;
        response["errmsg"] = "用户名或密码错误!";
        conn->send(response.dump());
    }
}
// 处理注册业务
void ChatService::reg(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    LOG_INFO << "do reg...";
    User user;
    user.setName(js["name"]);
    user.setPassword(js["password"]);
    json response;
    if(_userModel.insert(user))
    {
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0;
        response["id"] = user.getId();
    }
    else 
    {
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "注册失败!";
    }
    conn->send(response.dump());
}

// 客户端异常退出
void ChatService::clientCloseException(const TcpConnectionPtr& conn)
{
    User user;
    {
        std::lock_guard<std::mutex> lock(_connMutex);
        for (auto it = _userConnMap.begin(); it != _userConnMap.end(); ++it)
        {
            if (it->second == conn)
            {
                user.setId(it->first);
                _userConnMap.erase(it);
                break;
            }
        }
        _redis.unsubscribe(user.getId());
    }
    if (user.getId() != -1)
    {
        user.setState("offline");
        _userModel.updateState(user);
        LOG_INFO << "Client: " << conn->peerAddress().toIpPort() << " offline!";
    }
    conn->shutdown();
}

void ChatService::oneToOneChat(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    string msg = js["msg"];
    int id = js["toid"].get<int>(); 
    {
        // 找目标是否在线
        json response;
        response["msgid"] = ONE_CHAT_MSG;
        std::lock_guard<mutex> lock(_connMutex);
        response["id"] = js["id"];
        response["msg"] = js["msg"];
        response["time"] = js["time"];
        response["name"] = js["name"];
        if (_userConnMap.find(id) != _userConnMap.end())
        {
            _userConnMap[id]->send(js.dump());
        }
        else 
        {
            // 查是否在线
            User user = _userModel.query(id);
            if (user.getState() == "offline")
            {
                _offlineMsgModel.insert(id, js.dump());
            }
            else
            {
                _redis.publish(id, js.dump());
            }
        }
    }
}

void ChatService::reset()
{
    _userModel.resetState();
}

void ChatService::addFriend(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();
    json response;
    response["msgid"] = ADD_FRIEND_MSG_ACK;
    if (_friendModel.insert(userid, friendid))
    {
        response["errno"] = 0;
        response["msg"] = "添加好友成功!";
    }
    else
    {
        response["errno"] = 1;
        response["msg"] = "添加好友失败!";
    }
    conn->send(response.dump());
}
// 创建群组
void ChatService::createGroup(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    Group group;
    string groupname = js["groupname"];
    string groupdesc = js["groupdesc"];
    json response;
    response["msgid"] = CREATE_GROUP_MSG_ACK;
    group.setName(groupname);
    group.setDesc(groupdesc);
    if (_groupModel.insert(group))
    {
        // 群组加入自己
        _groupModel.addToGroup(group.getId(), js["id"].get<int>(), "creator");
        response["errno"] = 0;
        response["msg"] = "创建群组成功!";
        response["groupid"] = group.getId();
    }
    else
    {
        response["errno"] = 1;
        response["msg"] = "创建群组失败!";
    }
    conn->send(response.dump());
}
// 群聊
void ChatService::groupChat(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    int groupid = js["groupid"].get<int>();
    int senderid = js["id"].get<int>();
    string msg = js["msg"];
    json response;
    response["msgid"] = GROUP_CHAT_MSG;
    vector<int> memberids = _groupModel.queryGroupUsers(senderid, groupid);
    {
        std::lock_guard<std::mutex> lock(_connMutex);
        response["id"] = senderid;
        response["msg"] = msg;
        response["groupid"] = groupid;
        response["time"] = js["time"];
        response["name"] = js["name"];
        for (auto id : memberids)
        {
            if (_userConnMap.find(id) != _userConnMap.end())
            {
                _userConnMap[id]->send(response.dump());
            }
            else
            {
                // 不在线放入离线消息中
                if (_userModel.query(id).getState() == "offline")
                {
                    _offlineMsgModel.insert(id, response.dump());
                }
                else
                {
                    _redis.publish(id, response.dump());
                }
            }
        }
    }
}
// 加入群组
void ChatService::joinGroup(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    int userid = js["userid"].get<int>();
    int groupid = js["groupid"].get<int>();
    json response;
    if (_groupModel.addToGroup(groupid, userid, "normal"))
    {
        response["msgid"] = ADD_GROUP_MSG_ACK;
        response["errno"] = 0;
        response["msg"] = "加入群组成功!";
    }
    else
    {
        response["msgid"] = ADD_GROUP_MSG_ACK;
        response["errno"] = 1;
        response["msg"] = "加入群组失败!";
    }
    conn->send(response.dump());
}

void ChatService::handleRedisSubscribeMessage(int channel, string message)
{
    // 处理redis订阅的消息
    json js = json::parse(message);
    std::lock_guard<std::mutex> lock(_connMutex);
    if (_userConnMap.find(channel) != _userConnMap.end())
    {
        _userConnMap[channel]->send(message);
    }
    else
    {
        // 不在线放入离线消息中
        _offlineMsgModel.insert(channel, message);
    }
}