#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include <unordered_map>
#include <functional>
#include <muduo/net/TcpConnection.h>
#include <mutex>
using namespace std;
using namespace muduo::net;
using namespace muduo;
#include "json.hpp"
#include "usermodel.hpp"
#include "offlinemessagemodel.hpp"
#include "friendmodel.hpp"
#include "groupmodel.hpp"
#include "redis.hpp"
using json = nlohmann::json;

// 处理消息的事件回调
using MsgHandler = std::function<void(const TcpConnectionPtr& conn, json& js, Timestamp)>;

// 单例
class ChatService
{
public:
    static ChatService* getInstance();
    // 获取消息对应的处理器
    MsgHandler getMsgHandler(int msgid);
    // 客户端异常退出
    void clientCloseException(const TcpConnectionPtr& conn);
    void reset();
private:
    ChatService();
    // 处理登录业务
    void login(const TcpConnectionPtr& conn, json& js, Timestamp time);
    // 处理注册业务
    void reg(const TcpConnectionPtr& conn, json& js, Timestamp time);
    // 单聊
    void oneToOneChat(const TcpConnectionPtr& conn, json& js, Timestamp time);
    // 添加好友
    void addFriend(const TcpConnectionPtr& conn, json& js, Timestamp time);
    // 创建群组
    void createGroup(const TcpConnectionPtr& conn, json& js, Timestamp time);
    // 群聊
    void groupChat(const TcpConnectionPtr& conn, json& js, Timestamp time);
    // 加入群组
    void joinGroup(const TcpConnectionPtr& conn, json& js, Timestamp time);
    // 处理redis订阅的消息
    void handleRedisSubscribeMessage(int channel, string message);
private:
    unordered_map<int, MsgHandler> _msgHandlerMap;     // 消息处理映射表
    unordered_map<int, TcpConnectionPtr> _userConnMap; // 在线用户的连接-需要考虑线程安全
    UserModel _userModel;                              // 用户数据操作对象
    OfflineMsgModel _offlineMsgModel;
    FriendModel _friendModel;                          // 好友数据操作对象
    GroupModel _groupModel;                            // 群组数据操作对象

    std::mutex _connMutex;                             // 互斥锁
    Redis _redis;                                     // redis操作对象
};


#endif