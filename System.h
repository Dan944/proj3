#ifndef SYSTEM_H
#define SYSTEM_H

#include "User.h" // Corrected include path
#include <vector>
#include <string>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <vector>

class System {
public:
    std::vector<User*> allUsers;
    std::vector<User*> onlineUsers;
    std::vector<std::string> getAllUsers();
    void init();
    void who();
    void load_user();
    User user;
    std::vector<int> states = std::vector<int>(100, 0);
    User* findUser(const std::string& username);
    User* findUserFd(const int fd);
    void onlineUpdate();
    void who(int fd);
    void regist(int fd, std::string username, std::string password);
    void stats(int fd, const std::string name);
    void writeLine(int socketId, const std::string line);
    static void rtrim(std::string &s);
    void printAllUsers() const;
    // void login(int rec_sock, std::string username, std::string password);
    
    void startAutoSave();
    void stopAutoSave();
    void saveUserData(); 

    // std::atomic<bool> keepRunning;
    // std::thread autoSaveThread;
};

#endif // SYSTEM_H
