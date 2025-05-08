#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
using namespace muduo;
using namespace muduo::net;

class ChatServer {
public:
    // 初始化服务器
    ChatServer(EventLoop* loop, const InetAddress& listenAddr, const string& nameArg);
    // 启动服务器
    void start();
    // 服务器重置
    void reset();
private:
    // 回调上报连接相关事件(连接建立、连接断开)
    void onConnection(const TcpConnectionPtr& conn);
    // 回调上报消息相关事件(收到消息)
    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time);

private:
    TcpServer _server; // 服务器对象
    EventLoop *_loop;  // 事件循环对象
};

#endif