#ifndef GAMELIST_H
#define GAMELIST_H

#include "User.h" // Include the User class
#include <vector>
#include <string>
#include <ctime>
#include <iostream>

enum class GameStatus {
    InProgress,
    Completed
};

class GameList {
public:
    GameStatus status;            // Status of the game
    int gameID;                   // Unique identifier for the game
    // int requestResult;            // 0 is refused,1 is accepted
    User player1;                 // First player
    User player2;                 // Second player
    std::vector<User> observers;  // List of observers watching the game
    std::time_t *start_Time;      // Start time of the game
    std::time_t *end_Time;        // End time of the game

    // Constructor
    GameList(int id, const User& p1, const User& p2)
      : gameID(id), 
        player1(p1), 
        player2(p2), 
        start_Time(nullptr), 
        end_Time(nullptr), 
        status(GameStatus::InProgress) 
    {
        start_Time = new std::time_t(std::time(nullptr));
    }

    // Destructor
    ~GameList() {
        delete start_Time; // Ensure to delete allocated memory
        delete end_Time;
    }

    // Method to add an observer
    void addObserver(const User& observer) {
        observers.push_back(observer);
    }

    GameList(const GameList &other){
        status = other.status;
        gameID = other.gameID;
        player1 = other.player1;
        player2 = other.player2;
        observers = other.observers;
        start_Time = other.start_Time;
        end_Time = other.end_Time;
    }

};

#endif // GAMELIST_H
