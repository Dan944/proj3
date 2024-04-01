#include "GameRecall.h"
#include "System.h"
#include <chrono>
#include <unistd.h>

GameRecall::GameRecall()
{

}

GameRecall::GameRecall(User* player1, User* player2, int id)
    : player1(player1), 
    player2(player2), 
    gameID(id), 
    currentPlayer(false), 
    isGameOver(false)
{
    start_Time = new std::time_t(std::time(nullptr)); // Initialize start time to now
    end_Time = nullptr; 
    std::memset(checkerboard, 0, sizeof(checkerboard));
    move_step = 0;
}

bool GameRecall::addMove(int player, const std::string& move) {
    // startTurn();
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
            // endTurn();
            move_step++;
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

// Print the checkercheckerboard
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

bool GameRecall::isMoveCommand(const std::string command) {
    if (command.size() == 2) {
        char ch1 = command[0];
        char ch2 = command[1];
        return (ch1 == 'A' || ch1 == 'B' || ch1 == 'C') && (ch2 == '1' || ch2 == '2' || ch2 == '3');
    }
    return false;
}

bool GameRecall::isWin(int bw) {
    for (int i = 0; i < 3; i++) {
        if (checkerboard[i][0] == checkerboard[i][1] && checkerboard[i][0] == checkerboard[i][2] && checkerboard[i][0] == bw) {
            return true;
        }

        if (checkerboard[0][i] == checkerboard[1][i] && checkerboard[0][i] == checkerboard[2][i] && checkerboard[0][i] == bw) {
            return true;
        }
    }

    if (checkerboard[0][0] == checkerboard[1][1] && checkerboard[0][0] == checkerboard[2][2] && checkerboard[0][0] == bw) {
        return true;
    }

    if (checkerboard[0][2] == checkerboard[1][1] && checkerboard[0][2] == checkerboard[2][0] && checkerboard[0][2] == bw) {
        return true;
    }

    return false;
}

std::string GameRecall::getBoardAsString() const {
    std::ostringstream boardStr;

    // Print the column headers
    boardStr << "  1 2 3\n";
    for (int i = 0; i < 3; ++i) {
        // Print the row label ('a' + row index)
        boardStr << static_cast<char>('a' + i) << ' ';
        for (int j = 0; j < 3; ++j) {
            char symbol = '.';
            if (checkerboard[i][j] == 1) symbol = 'X';
            else if (checkerboard[i][j] == 2) symbol = 'O';
            boardStr << symbol << " ";
        }
        boardStr << "\n"; // Use "\n" instead of std::endl to avoid flushing the buffer
    }

    return boardStr.str();
}

bool GameRecall::isDraw() const {
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            if (checkerboard[i][j] == 0) {
                // Found an empty cell, so it's not a draw
                return false;
            }
        }
    }

    return true;
}
void wrtel(int socketId, const std::string line) {
    std::string temp = line + '\n';
    write(socketId, temp.c_str(), temp.size());
}

void GameRecall::endGame(int mod) {
    if (mod == 0){
        wrtel(player1->getSockId(), "It's a draw.");
        wrtel(player2->getSockId(), "It's a draw.");
        player2->writef("");
        isGameOver = true;
    }
    else if (mod == 1){
        wrtel(player1->getSockId(), "You win!");
        player1->win1();
        wrtel(player2->getSockId(), "You lose.");
        player2->loss1();
        player2->writef("");
        isGameOver = true;
    }
    else if (mod == 2){
        wrtel(player2->getSockId(), "You win!");
        player2->win1();
        wrtel(player1->getSockId(), "You lose.");
        player1->loss1();
        player1->writef("");
        isGameOver = true;
    }

}

// void GameRecall::manageGame(int fd, GameRecall *game) {
//     printf("Manage Game Start!\n");
//     System sys;
//     bool gameover = false;
//     std::string moveCommand;
//     int c_Player = 1;
//     sys.writeLine(game->player1->getSockId(),"You go first\n");
//     sys.writeLine(game->player2->getSockId(),"You go next\n");
//     printf("cPlayer is %d\n",c_Player);
//     std::cout << "currentPlayer is : " << currentPlayer << endl;
    
//     while (!isGameOver) {
//         int currentPlayerSockId = game->player1->getSockId();
        
//         // Request a move from the current player
//         std::cout << "currentPlayerSockId is : " << currentPlayerSockId << endl;
//         std::string boardState = getBoardAsString();
//         sys.writeLine(game->player1->getSockId(),boardState);
//         sys.writeLine(game->player2->getSockId(),boardState);
//         if (currentPlayer == 0){
//             sys.writeLine(game->player2->getSockId(),"Wait for your turn.");
//             sys.writeLine(game->player1->getSockId(),"It's your turn.");
//             moveCommand = sys.readLine(game->player1->getSockId());
//             sys.rtrim(moveCommand);
//         } else {
//             sys.writeLine(game->player1->getSockId(),"Wait for your turn.");
//             sys.writeLine(game->player2->getSockId(),"It's your turn.");
//             moveCommand = sys.readLine(game->player2->getSockId());
//             sys.rtrim(moveCommand);
//         }

//         // Try to add the move
//         if (addMove(c_Player, moveCommand)) {
//             // Successfully added the move. Send updated board to both players.
//             printf("addmove successful\n");
//             std::string boardState = getBoardAsString();
//             sys.writeLine(game->player1->getSockId(), boardState);
//             sys.writeLine(game->player2->getSockId(), boardState);
            
//             // Check for win/draw condition
//             if (isWin(currentPlayer ? 1 : 2)) {
//                 sys.writeLine(currentPlayerSockId, "You win!");
//                 sys.writeLine(currentPlayer ? game->player2->getSockId() : game->player1->getSockId(), "You lose.");
//                 gameover = true;
//             }

//             if (isDraw() == true){
//                 sys.writeLine(game->player1->getSockId(), "It's a draw.");
//                 sys.writeLine(game->player2->getSockId(), "It's a draw.");
//                 break;
//             }
            
//             // Here, add logic to check for a draw if necessary.

//             if (gameover) {
//                 isGameOver = true;
//                 break;
//             }
            
//             // Switch turns
//             game->currentPlayer = !game->currentPlayer;
//             if(c_Player == 1){
//                 c_Player = 2;
//             } else {
//                 c_Player = 1;
//             }
//             printf("cPlayer is %d\n",c_Player);
//             std::cout << "currentPlayer is : " << currentPlayer << endl;
//         } else {
//             // Invalid move. Inform the current player.
//             sys.writeLine(currentPlayerSockId, "Invalid move. Please try again.");
//         }
//     }
// }

// void GameRecall::handleMatchRequest(int fd, User* matchUser, User* requestingUser) {
//     System sys;
//     std::string answer = sys.readLine(fd);
//     sys.rtrim(answer);

//     if (answer == "yes") {
//         // Assuming we have function to initialize GameRecall with both players
//         GameRecall* game = startGame(matchUser, requestingUser);
//         std::cout << "handle Player1 is : " << game->player1->getSockId() << endl;
//         std::cout << "handle Player2 is : " << game->player2->getSockId() << endl;
//         manageGame(fd, game);
//     } else if (answer == "no") {
//         sys.writeLine(fd, "User refused your request.");
//     }
// }

// GameRecall* GameRecall::handleMatchRequest(int fd, User* matchUser, User* requestingUser, int gameID) {
//     System sys;
//     std::string answer = sys.readLine(fd);
//     sys.rtrim(answer);

//     if (answer == "yes") {
//         // Assuming we have function to initialize GameRecall with both players
//         GameRecall *game = new GameRecall(matchUser, requestingUser, gameID);
//         game->player1->setCurrentGameID(gameID);
//         game->player2->setCurrentGameID(gameID);
//         sys.writeLine(game->player1->getSockId(),"You go first\n");
//         sys.writeLine(game->player2->getSockId(),"You go next\n");
//         return game;
//     } else if (answer == "no") {
//         sys.writeLine(fd, "User refused your request.");
//     }

//     return nullptr;
// }

GameRecall* GameRecall::handleMatchRequest(int fd, User* matchUser, User* requestingUser, int gameID) {
    System sys;
    // Assuming we have function to initialize GameRecall with both players
    GameRecall *game = new GameRecall(matchUser, requestingUser, gameID);
    game->player1->setCurrentGameID(gameID);
    game->player2->setCurrentGameID(gameID);
    // startTurn(game);
    sys.writeLine(game->player1->getSockId(),"You go first\n");
    sys.writeLine(game->player2->getSockId(),"You go next\n");
    return game;
}


GameRecall* GameRecall::startGame(User* player1, User* player2) {
    // Initialize the game instance
    GameRecall *game = new GameRecall(player1, player2, 1);
    std::cout << "start in Player1 is : " << player1->getSockId() << endl;
    std::cout << "start in Player2 is : " << player2->getSockId() << endl;
    // Initial setup if needed
    return game;
}

// void GameRecall::startTurn() {
//     startTurnTime = std::chrono::steady_clock::now();
// }

// void GameRecall::endTurn() {
//     auto endTurnTime = std::chrono::steady_clock::now();
//     auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(endTurnTime - startTurnTime);

//     // Deduct elapsed time from the current player's remaining time
//     if (!currentPlayer) {
//         player1TimeLeft -= elapsed;
//         if (player1TimeLeft.count() <= 0) {
//             // Player 1 has run out of time
//             declareWinner(player2);
//             return;
//         }
//     } else {
//         player2TimeLeft -= elapsed;
//         if (player2TimeLeft.count() <= 0) {
//             // Player 2 has run out of time
//             declareWinner(player1);
//             return;
//         }
//     }

//     startTurn();
// }

// void GameRecall::declareWinner(User* winner){
//     isGameOver = true;
    
// }