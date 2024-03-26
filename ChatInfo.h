#ifndef CHATINFO_H
#define CHATINFO_H

#include "User.h"
#include <chrono>
#include <memory>
#include <string>

class ChatInfo {
public:
    int gameid;
    std::string chatContent;
    User *rec_User; // Use smart pointers for user references
    User *send_User;
    std::chrono::system_clock::time_point time; // Use chrono's time_point for time representation
    bool isRecv = false; // Initialized default value
    bool isRead = false; // Initialized default value

    // Constructor
    ChatInfo(int gameId, const std::string& content, User *sender, User *receiver)
      : gameid(gameId), 
        chatContent(content), 
        rec_User(receiver), 
        send_User(sender), 
        time(std::chrono::system_clock::now()) 
    {

    }

    ChatInfo(const ChatInfo &other){
        gameid = other.gameid;
        chatContent = other.chatContent;
        rec_User = other.rec_User;
        send_User = other.send_User;
        time = other.time;
    }

};

#endif // CHATINFO_H
