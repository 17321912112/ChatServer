#include "group.hpp"
#include "user.hpp"
#include "public.hpp"
#include <json.hpp>
using json = nlohmann::json;
#include <muduo/net/TcpClient.h>
#include <muduo/net/EventLoop.h>
using namespace muduo;
using namespace muduo::net;
#include <iostream>
#include <string>
#include <vector>
using namespace std;

User user;

void userWindow(const TcpConnectionPtr &conn) 
{
    cout << "Welcome user id:"<< user.getId() << " name:" << user.getName() <<"!" << endl;
    // 1.显示好友列表 2.显示群组列表 3.发送消息 4.添加好友 5.创建群组 6.加入群组
    cout << "1. Show friend list" << endl;
    cout << "2. Show group list" << endl;
    cout << "3. Send message" << endl;
    cout << "4. Add friend" << endl;
    cout << "5. Create group" << endl;
    cout << "6. Join group" << endl;
    cout << "7. Logout" << endl;
    int choice;
    cin >> choice;
    switch (choice) 
    {
        case 1:
            cout << "Show friend list selected." << endl;
            break;
        case 2:
            cout << "Show group list selected." << endl;
            break;
        case 3:
            cout << "Send message selected." << endl;
            break;
        case 4:
            cout << "Add friend selected." << endl;
            break;
        case 5:
            cout << "Create group selected." << endl;
            break;
        case 6:
            cout << "Join group selected." << endl;
            break;
        case 7:
            user.setState("offline");
    }
}

void onMessage(const TcpConnectionPtr &conn, Buffer *buffer, Timestamp time) 
{
    string msg = buffer->retrieveAllAsString();
    cout << "Received message: " << msg << endl;
    json j = json::parse(msg);
    // 处理服务器返回的消息
    int msgId = j["msgid"];
    switch (msgId) 
    {
        case LOGIN_MSG_ACK: 
        {
            if (j["errno"] == 0) 
            {
                cout << "Login successful! User ID: " << j["id"] << endl;
                user.setId(j["id"]);
                user.setName(j["name"]);
                user.setState("online");
                userWindow(conn);
            } else {
                cout << "Login failed! Error: " << j["errmsg"] << endl;
            }
            break;
        }
        case REG_MSG_ACK: 
        {
            if (j["errno"] == 0) 
            {
                cout << "Registration successful! User ID: " << j["id"] << endl;
            } else {
                cout << "Registration failed! Error: " << j["errmsg"] << endl;
            }
            break;
        }
        default:
            cout << "Unknown message ID: " << msgId << endl;
    }
}


void login(const TcpConnectionPtr &conn) 
{
    cout << "Please enter your username and password." << endl;
    string username, password;
    cout << "Enter userId: ";
    cin >> username;
    cout << "Enter password: ";
    cin >> password;
    
    // 发送登录请求到服务器
    json j;
    j["msgid"] = LOGIN_MSG;
    j["id"] = atoi(username.c_str());
    j["password"] = password;
    
    // 发送消息
    conn->send(j.dump());
}

void registerUser(const TcpConnectionPtr &conn) 
{
    cout << "Please enter your username and password." << endl;
    string username, password;
    cout << "Enter username: ";
    cin >> username;
    cout << "Enter password: ";
    cin >> password;
    
    // 发送注册请求到服务器
    json j;
    j["msgid"] = REG_MSG;
    j["name"] = username;
    j["password"] = password;
    
    // 发送消息
    conn->send(j.dump());
}

// 主界面
void mainMenu(const TcpConnectionPtr &conn) 
{
    cout << "Welcome to the chat client!" << endl;
    cout << "1. Login" << endl;
    cout << "2. Register" << endl;
    cout << "3. Exit" << endl;
    cout << "Please select an option: ";
    int option;
    cin >> option;
    switch (option) 
    {
        case 1:
            cout << "Login selected." << endl;
            login(conn);
            break;
        case 2:
            cout << "Register selected." << endl;
            registerUser(conn);
            break;
        case 3:
            conn->forceClose();
            exit(0);
            break;
        default:
            cout << "Invalid option. Please try again." << endl;
            mainMenu(conn);
    }
}

int main(int argc, char *argv[]) 
{
    if (argc < 3) 
    {
        cerr << "example: ./client <ip> <port>" << endl;
        return 1;
    }
    string ip = argv[1];
    string port = argv[2];
    // 1. 创建客户端对象
    EventLoop loop;
    TcpClient client(&loop, InetAddress(ip, stoi(port)), "Client");
    client.connect();
    // 2. 连接服务器
    client.setConnectionCallback
    (
        [](const TcpConnectionPtr &conn) {
            if (conn->connected()) 
            {
                cout << "Connected to server!" << endl;
                mainMenu(conn);
            } else {
                cout << "Disconnected from server!" << endl;
            }
    });
    client.setMessageCallback(onMessage);
    // 3. 设置消息回调函数
    loop.loop();
}