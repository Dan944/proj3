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
    User *user = new User(username, password);
    for (int i=0; i<=1024; i++){
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

User* System::findUser(const std::string& username) {
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
                << blockedNamesStream.str() << std::endl;
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

// void System::login(int rec_sock, std::string username, std::string password) {
// 	char buf[1024] = "username(guest):";
// 	System::rtrim(password);
// 	// string dusername = username.substr(0, username.length() - 2);
// 	// string dpassword = password.substr(0, password.length() - 2);
// 	User *user = System::findUser(username);
// 	if (user == nullptr) {
// 		System::writeLine(rec_sock, "Incorrect username");
// 		states[rec_sock] = 0;
// 		write(rec_sock, buf, strlen(buf));
// 	} else if (user->password==password){
// 		if (user->login==true) {
// 			states[user->sockId]=-1;
// 			char q[100];
// 			strcpy(q,"sorry, your account has been login in other place, enter any button to quit\n");
// 			write(user->sockId,q,strlen(q));
// 			user->sockId = rec_sock;
// 			states[rec_sock] = 3;
// 			user->cmd = 0;
// 		} else {
// 			user->login = true;
// 			user->sockId = rec_sock;
// 			states[rec_sock] = 3;
// 		}
// 	} else {
// 		writeLine(rec_sock, "Incorrect password");
// 		std::cout << "user.password = "<<user->password<<std::endl;
// 		std::cout << "password = "<<password<<std::endl;
// 		states[rec_sock] = 0;
// 		write(rec_sock, buf, strlen(buf));
// 	}
// }