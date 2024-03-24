#ifndef GAMERECALL_H
#define GAMERECALL_H

#include "User.h"
#include <vector>
#include <string>
#include <ctime>
#include <iostream>

class GameRecall {
public:
    std::time_t* start_Time;
    std::time_t* end_Time;
    User* p_User;
    int gameID;
    float checkerboard[3][3]; // Board for the game, assuming it's a game like tic-tac-toe

    // Constructor
    GameRecall(User* player, int id) 
      : p_User(player), 
        gameID(id) 
    {
        start_Time = new std::time_t(std::time(nullptr)); // Initialize start time to now
        end_Time = nullptr; // End time is not set until the game is finished

        // Initialize checkerboard with 0s, assuming 0 represents an empty cell
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                checkerboard[i][j] = 0.0f;
            }
        }
    }

    // Destructor to properly manage dynamic memory
    ~GameRecall() {
        delete start_Time;
        delete end_Time;
    }

    GameRecall(const GameRecall &other){
        start_Time = other.start_Time;
        end_Time = other.end_Time;
        p_User = other.p_User;
        gameID = other.gameID;
        checkerboard[3][3] = other.checkerboard[3][3];
    }

    bool addMove(int player, int row, int col);
    void printBoard() const;
};

#endif // GAMERECALL_H
