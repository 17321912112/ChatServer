#include "chatserver.hpp"
#include <muduo/base/Logging.h>
#include "json.hpp"
using json = nlohmann::json;
#include "public.hpp"
#include "chatservice.hpp"


// 初始化服务器
ChatServer::ChatServer(EventLoop* loop, const InetAddress& listenAddr, const string& nameArg)
    :_server(loop, listenAddr, nameArg)
    , _loop(loop)
{
    _server.setConnectionCallback(
        std::bind(&ChatServer::onConnection, this, ::_1)
    );
    _server.setMessageCallback(
        std::bind(&ChatServer::onMessage, this, ::_1, ::_2, ::_3)
    );
    _server.setThreadNum(4);

}
// 启动服务器
void ChatServer::start()
{
    // 启动服务器
    _server.start();
}

// 回调上报连接相关事件(连接建立、连接断开)
void ChatServer::onConnection(const TcpConnectionPtr& conn)
{
    if (conn->disconnected())
    {
        ChatService::getInstance()->clientCloseException(conn);
    }
}
// 回调上报消息相关事件(收到消息)
void ChatServer::onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time)
{
    string msg = buf->retrieveAllAsString();
    LOG_INFO << conn->name() << "send:" << msg;
    json js = json::parse(msg);
    LOG_DEBUG << "msgid:" << js["msgid"].get<int>();
    auto handler = ChatService::getInstance()->getMsgHandler(js["msgid"].get<int>());
    handler(conn, js, time);
}

void ChatServer::reset()
{
    ChatService::getInstance()->reset();
}