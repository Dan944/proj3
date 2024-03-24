#ifndef SYSTEM_H
#define SYSTEM_H

#include "User.h" // Corrected include path
#include <vector>
#include <string>
#include <cstring>

class System {
public:
    std::vector<User*> allUsers;
    std::vector<std::string> getAllUsers();
    void init();
    void who();
    void load_user();
    User user;
    User* findUser(const std::string username);
    void startAutoSave();
    void stopAutoSave();
    void saveUserData(); 
};

#endif // SYSTEM_H
