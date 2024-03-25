#include "System.h"
#include "User.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <chrono>
#include <algorithm>
#include <cctype>
#include <vector>

// Gets all usernames from the system's user list
std::vector<std::string> System::getAllUsers() {
    std::vector<std::string> usernames;
    for (auto& user : allUsers) {
        usernames.push_back(user->username);
    }
    return usernames;
}

void System::rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// Initializes the system, possibly by loading user data
void System::init() {
    load_user();
}

void System::writeLine(int socketId, const std::string line) {
    std::string temp = line + '\n';
    write(socketId, temp.c_str(), temp.size());
}

// Displays information about who is currently online, placeholder implementation
void System::who(int fd) {
    onlineUpdate();
    char out1[100];
    sprintf(out1,"Total %i user(s) online:\n", onlineUsers.size());
    std::string out2;
    for (User* u : onlineUsers) {
        out2 += u->username;
        out2 += " ";
    }
    write(fd, out1, strlen(out1));
    writeLine(fd, out2);
}

void System::regist(int fd, std::string username, std::string password) {
    User *user1 = findUser(username);
    if (user1 != nullptr ){
        writeLine(fd, "This name has been registed, please user another name");
        return;
    }
    rtrim(password);
    User *user = new User(username, password);
    for (int i=1; i<=1024; i++){
        bool ava = true;
        for (User *u : allUsers) {
            if (u->id == i) {
                ava = false;
            }
        }
        if (ava) {
            user->id = i;
            break;
        }
    }
    allUsers.push_back(user);
    System::saveUserData();
    writeLine(fd, "registed sucessful");
}

// Loads user data from a file
void System::load_user() {
    std::string filePath = "data/user"; // Adjust the path as necessary
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open user file." << std::endl;
    }
    std::string line;
    std::getline(file, line);
    while (std::getline(file, line)) {
        // User,Password,ID,SockID,Information,Rating,ObserverID,PlayID,cmd,Win,Loss,isQuiet,isLogin,BlockName
        printf("******************************loop*************************************\n");
        int count = 0;
        User *user = new User();
        std::stringstream ss(line);
        std::string item;
        std::vector<std::string> tokens;
        while (std::getline(ss, item, ',')) {
            tokens.push_back(item);
        }
        tokens.push_back("");
        user->setUsername(tokens[0]);
        printf("0\n");
        user->setPassword(tokens[1]);
        printf("1\n");
        user->setId(std::stoi(tokens[2]));
        printf("2\n");
        user->setInformation(tokens[3]);
        printf("4\n");
        user->setRating(std::stof(tokens[4]));
        printf("5\n");
        user->setWin(std::stoi(tokens[5]));
        printf("9\n");
        user->setLoss(std::stoi(tokens[6]));
        printf("10\n");
        user->setQuiet(std::stoi(tokens[7]));
        printf("11\n");
        std::istringstream blockStream(tokens[8]); // Use tokens[8] to create a stream
        printf("13\n");
        std::string blockName;
        while (std::getline(blockStream, blockName, ';')) {
            user->blocked_names.push_back(blockName);
        }
        allUsers.push_back(user);
    }
    file.close();
}

User* System::findUser(std::string username) {
    rtrim(username);
    for (User* u : allUsers) {
        if (u->getUsername() == username) {
            return u;
        }
    }
    return nullptr;
}

User* System::findUserFd(const int fd) {
    for (User *u : allUsers) {
        if (u->sockId == fd) {
            return u;
        }
    }
    return nullptr;
}

void System::onlineUpdate(){
    onlineUsers.clear();
    for (User* u : allUsers) {
        if (u->login) { // If the user is marked as online
            onlineUsers.push_back(u);
        }
    }
}

void System::printAllUsers() const {
    std::cout << "All users:\n";
    for (const User* user : allUsers) {
        std::cout << user->getUsername() << std::endl; // Assuming User class has a getUsername() method
    }
}


// void System::startAutoSave() {
//     autoSaveThread = std::thread([this](){
//         while (keepRunning.load()) {
//             std::this_thread::sleep_for(std::chrono::seconds(5));
//             saveUserData();
//         }
//     });
// }

// void System::stopAutoSave() {
//     keepRunning.store(false);
//     if (autoSaveThread.joinable()) {
//         autoSaveThread.join();
//     }
// }

void System::saveUserData() {
    std::string filePath = "data/user";
    std::ofstream outFile(filePath, std::ofstream::trunc); // Open the file in truncate mode to overwrite

    if (!outFile.is_open()) {
        std::cerr << "Failed to open file for writing: " << filePath << std::endl;
        return;
    }

    outFile << "User" << ","
            << "Password" << ","
            << "ID" << ","
            << "Information" << ","
            << "Rating" << ","
            << "Win" << ","
            << "Loss" << ","
            << "isQuiet" << ","
            << "BlockName" << std::endl;

    for (const User* user : allUsers) {
        // Convert blocked_ids vector to a string
        printf("loop\n");
        std::stringstream blockedNamesStream;
        for (size_t i = 0; i < user->getBlockedNames().size(); ++i) {
            blockedNamesStream << user->getBlockedNames()[i];
            if (i < user->getBlockedNames().size() - 1) blockedNamesStream << ";"; // delimiter for blocked_ids
        }

        // Writing user data, converting boolean to string for clarity
        outFile << user->getUsername() << ","
                << user->getPassword() << ","
                << user->getId() << ","
                << user->getInformation() << ","
                << user->getRating() << ","
                << user->getWin() << ","
                << user->getLoss() << ","
                << (user->isQuiet() ? true : false) << ","
                << blockedNamesStream.str() <<std::endl;
    }

    outFile.close();
    std::cout << "User data saved successfully." << std::endl;
}

void System::stats(int fd, const std::string name){
    char statsStr[1024];
    User *user = System::findUser(name);
    if (user == nullptr){
        writeLine(fd,"User does not exist.");
        return;
    }
    int i=0;
    const char* boolStr1 = user->quiet ? "Yes" : "No";
    const char* boolStr2 = user->login ? "Online" : "Offline";
    std::string blockStr = "";
    char loginStr[100] = "";
    sprintf(loginStr,"%s is currently %s.",user->username.c_str(),boolStr2);
    for ( i = 0; i < user->blocked_names.size(); i++) {
        blockStr += user->blocked_names[i];
        blockStr += " ";
    }
    sprintf(statsStr,"User: %s\nInfo: %s\nRating: %f\nWins: %d, Loses: %d\nQuiet: %s\nBlocked users: %s\n\n%s\n",
        user->username.c_str(),user->information.c_str(),user->rating,user->win,user->loss,boolStr1,blockStr.c_str(),loginStr);
    write(fd,statsStr,strlen(statsStr));
}

void System::help(int fd){
    std::string helps = 
        "Commands supported:\n"
        "  who                     # List all online users\n"
        "  stats [name]            # Display user information\n"
        "  game                    # list all current games\n"
        "  observe <game_num>      # Observe a game\n"
        "  unobserve               # Unobserve a game\n"
        "  match <name> <b|w> [t]  # Try to start a game\n"
        "  <A|B|C><1|2|3>          # Make a move in a game\n"
        "  resign                  # Resign a game\n"
        "  refresh                 # Refresh a game\n"
        "  shout <msg>             # shout <msg> to every one online\n"
        "  tell <name> <msg>       # tell user <name> message\n"
        "  kibitz <msg>            # Comment on a game when observing\n"
        "  ' <msg>                 # Comment on a game\n"
        "  quiet                   # Quiet mode, no broadcast messages\n"
        "  nonquiet                # Non-quiet mode\n"
        "  block <id>              # No more communication from <id>\n"
        "  unblock <id>            # Allow communication from <id>\n"
        "  listmail                # List the header of the mails\n"
        "  readmail <msg_num>      # Read the particular mail\n"
        "  deletemail <msg_num>    # Delete the particular mail\n"
        "  mail <id> <title>       # Send id a mail\n"
        "  info <msg>              # change your information to <msg>\n"
        "  passwd <new>            # change password\n"
        "  exit                    # quit the system\n"
        "  quit                    # quit the system\n"
        "  help                    # print this message\n"
        "  ?                       # print this message";
    writeLine(fd, helps);
}
void System::info(int fd, char* buf){
    char *token = strchr(buf, ' '); 
    int argCount = 0;
    if (token != NULL) {
        User *user = findUserFd(fd);
        token++;
        char *msg = strchr(token, '\n');
        if (msg != NULL) {
            *msg = '\0';
        }
        user->information = token;
    }
    else {
        writeLine(fd,"Please enter information as info <msg>");
    }
}