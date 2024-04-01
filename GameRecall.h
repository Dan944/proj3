#ifndef GAMERECALL_H
#define GAMERECALL_H

#include "User.h"
#include <vector>
#include <string>
#include <ctime>
#include <iostream>
#include <chrono>
#include <unistd.h>

class GameRecall {
public:
    User* player1;
    User* player2;
    int gameID;
    bool currentPlayer;
    bool isGameOver;
    std::time_t* start_Time;
    std::time_t* end_Time;
    float checkerboard[3][3]; // Board for the game, assuming it's a game like tic-tac-toe
    std::chrono::steady_clock::time_point currentTurnTime;
    std::chrono::seconds timeLimit{600}; // 600 seconds = 10 minutes
    std::chrono::seconds player1TimeLeft{600};
    std::chrono::seconds player2TimeLeft{600};
    int move_step;

    // Constructor
    GameRecall(User* player1, User* player2, int id);

    // Destructor to properly manage dynamic memory
    ~GameRecall() {
        delete start_Time;
        delete end_Time;
    }

    GameRecall();

    GameRecall(const GameRecall &other){
        player1 = other.player1;
        player2 = other.player2;
        currentPlayer = other.currentPlayer;
        isGameOver = other.isGameOver;
        start_Time = other.start_Time;
        end_Time = other.end_Time;
        gameID = other.gameID;
        checkerboard[3][3] = other.checkerboard[3][3];
    }

    bool addMove(int player, const std::string& move);
    void printBoard() const;
    void manageGame(int fd, GameRecall *game);
    bool isMoveCommand(const std::string command);
    bool isWin(int bw);
    std::string getBoardAsString() const;
    GameRecall* handleMatchRequest(int fd, User* matchUser, User* requestingUser, int gameID);
    GameRecall* startGame(User* player1, User* player2);
    bool isDraw() const;
    void startTurn();
    User* endTurn();
    void endGame(int mod);
};

#endif // GAMERECALL_H
