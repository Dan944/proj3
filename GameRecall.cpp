#include "GameRecall.h"


bool GameRecall::addMove(int player, const std::string& move) {
    if (move.length() != 2) return false; // Ensure move is exactly 2 characters long

    // Convert 'a', 'b', 'c' to row index 0, 1, 2
    int row = move[0] - 'a';
    // Convert '1', '2', '3' to column index 0, 1, 2
    int col = move[1] - '1';

    // Check for valid row and column
    if (row >= 0 && row < 3 && col >= 0 && col < 3) {
        // Additional check if the position is already taken
        if (checkerboard[row][col] == 0) {
            checkerboard[row][col] = player; // Player 1 or 2
            currentPlayer = !currentPlayer; // Switch turn
            return true;
        } else {
            std::cout << "This position is already taken.\n";
        }
    } else {
        std::cout << "Invalid move. Please try again.\n";
    }

    return false;
}

// Print the checkerboard
void GameRecall::printBoard() const {
    // Print the column headers
    std::cout << "  1 2 3\n";
    for (int i = 0; i < 3; ++i) {
        // Print the row label ('a' + row index)
        std::cout << static_cast<char>('a' + i) << ' ';
        for (int j = 0; j < 3; ++j) {
            char symbol = '.';
            if (checkerboard[i][j] == 1) symbol = 'X';
            else if (checkerboard[i][j] == 2) symbol = 'O';
            std::cout << symbol << " ";
        }
        std::cout << std::endl;
    }
}