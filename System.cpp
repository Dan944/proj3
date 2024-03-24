#include "System.h"
#include "User.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <chrono>

// Gets all usernames from the system's user list
std::vector<std::string> System::getAllUsers() {
    std::vector<std::string> usernames;
    for (auto& user : allUsers) {
        usernames.push_back(user->username);
    }
    return usernames;
}

// Initializes the system, possibly by loading user data
void System::init() {
    load_user();
}

// Displays information about who is currently online, placeholder implementation
void System::who() {
    std::cout << "Online users:" << std::endl;
    for (auto& user : allUsers) {
        if (user->login) {
            std::cout << user->username << std::endl;
        }
    }
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

User* System::findUser(const std::string username) {
        for (User *u : allUsers) {
			std::cout << "data:" <<u->username << "|| this:" << username <<std::endl;
			std::cout << "data:" <<u->username.length() << "|| this:" << username.length() <<std::endl;
            if (u->username == username) {
				std::cout << "finded\n";
                return u;
            }
        }
        return nullptr;
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

