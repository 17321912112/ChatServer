#ifndef REDIS
#define REDIS
#include <hiredis/hiredis.h>
#include <iostream>
#include <string>
#include <functional>
using namespace std;

class Redis
{
public:
    Redis();
    ~Redis();
    // 连接redis
    bool connect();
    // 发布消息
    bool publish(int channel, const string &message);
    // 订阅消息
    bool subscribe(int channel);
    // 取消订阅
    bool unsubscribe(int channel);
    // 设置消息回调函数
    void init_notify_handler(function<void(int, string)> handler);
    // 在独立线程中接收订阅通道中的消息
    void observer_channel_message();
private:
    redisContext *_publish_context;
    redisContext *_subcribe_context;
    function<void(int, string)> _notify_message_handler;
};

#endif