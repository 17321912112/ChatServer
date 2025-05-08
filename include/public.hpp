#ifndef PUBLIC_H
#define PUBLIC_H
/**
 * server 和 client 公用头文件
 */

enum EnMsgType {
    LOGIN_MSG = 1,   // 登录
    REG_MSG,         // 注册
    ONE_CHAT_MSG,        // 聊天消息
    ADD_FRIEND_MSG,  // 添加好友
    CREATE_GROUP_MSG,    // 创建群组
    ADD_GROUP_MSG,  // 加入群组
    GROUP_CHAT_MSG, // 群聊

    REG_MSG_ACK,     // 注册响应
    LOGIN_MSG_ACK,   // 登录响应
    ONE_CHAT_MSG_ACK,    // 聊天消息响应
    ADD_FRIEND_MSG_ACK,   // 添加好友响应
    CREATE_GROUP_MSG_ACK, // 创建群组响应
    ADD_GROUP_MSG_ACK, // 加入群组响应
    GROUP_CHAT_MSG_ACK,  // 群聊消息响应
    LOGINOUT_MSG,   // 退出登录
};


#endif