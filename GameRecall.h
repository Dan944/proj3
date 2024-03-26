#ifndef GAMERECALL_H
#define GAMERECALL_H

#include "User.h"
#include <vector>
#include <string>
#include <ctime>
#include <iostream>

class GameRecall {
public:
    User* player1;
    User* player2;
    int gameID;
    bool currentPlayer;
    std::time_t* start_Time;
    std::time_t* end_Time;
    float checkerboard[3][3]; // Board for the game, assuming it's a game like tic-tac-toe

    // Constructor
    GameRecall(User* player1, User* player2, int id)
        : player1(player1), 
        player2(player2), 
        gameID(id), 
        currentPlayer(false) 
    {
        start_Time = new std::time_t(std::time(nullptr)); // Initialize start time to now
        end_Time = nullptr; 
        std::memset(checkerboard, 0, sizeof(checkerboard));
    }

    // Destructor to properly manage dynamic memory
    ~GameRecall() {
        delete start_Time;
        delete end_Time;
    }

    GameRecall(const GameRecall &other){
        player1 = other.player1;
        player2 = other.player2;
        currentPlayer = other.currentPlayer;
        start_Time = other.start_Time;
        end_Time = other.end_Time;
        gameID = other.gameID;
        checkerboard[3][3] = other.checkerboard[3][3];
    }

    bool addMove(int player, const std::string& move);
    void printBoard() const;
    void startGame(User* player1, User* player2, int id);
};

#endif // GAMERECALL_H
