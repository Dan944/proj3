#include "GameRecall.h"


bool GameRecall::addMove(int player, int row, int col) {
    if (row >= 0 && row < 3 && col >= 0 && col < 3 && checkerboard[row][col] == 0) {
        checkerboard[row][col] = player; // Player 1 or 2
        return true;
    }
    return false;
}

// Print the checkerboard
void GameRecall::printBoard() const {
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            char symbol = '.';
            if (checkerboard[i][j] == 1) symbol = 'X';
            else if (checkerboard[i][j] == 2) symbol = 'O';
            std::cout << symbol << " ";
        }
        std::cout << std::endl;
    }
}