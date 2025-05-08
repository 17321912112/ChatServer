#include "chatservice.hpp"
#include "chatserver.hpp"
#include <signal.h>
#include <iostream>
// 服务器ctrl+c结束后，重置user的状态信息
void resetHandler(int)
{
    ChatService::getInstance()->reset();
    exit(0);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: ./chatserver <port>" << std::endl;
        return 1;
    }
    int port = atoi(argv[1]);
    signal(SIGINT, resetHandler);
    EventLoop loop;
    InetAddress addr(port);
    ChatServer server(&loop, addr, "ChatServer");
    server.start();
    loop.loop();
    return 0;
}