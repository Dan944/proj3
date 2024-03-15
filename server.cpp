#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

int PORT = 8888;

const int MAX_NUM_BLOCKED = 5;

const int BLACK = 1;
const int WHITE = 2;

/** class for a user*/
struct User {
    string username; /* id of user*/
    string password;

    string information; /* personal information*/

    vector<string> blocked_names;

    bool quiet; /* whether this user is quiet*/
    bool login; /* whether this user is online*/
    string observeId;
    string playId;
    int sockId;

    User(string username, string password) {
        this->username = username;
        this->password = password;
        information = username;
        quiet = false;
        login = false;
        sockId = -1;
    }

    bool isBlock(string uname) {
        for (string s : blocked_names) {
            if (s == uname) {
                return true;
            }
        }
        return false;
    }
};

/** message data structure*/
struct Message {
    string form;
    string to;
    string message_body;
    int message_type;
    bool read;
};

struct Game {
    string id;
    int board[3][3]; //0 empty, 1 black, 2 white
    int turn;

    string blackUsername;
    string whiteUsername;

    int blackTime;
    int whiteTime;

    bool gameover;
    string winner;

    Game(int idInt, const string blackName, const string whiteName, int totalTime) {
        blackUsername = blackName;
        whiteUsername = whiteName;
        blackTime = totalTime;
        whiteTime = totalTime;
        turn = BLACK;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                board[i][j] = 0;
            }
        }

        gameover = false;
    }

    bool isWin(int bw) {
        for (int i = 0; i < 3; i++) {
            if (board[i][0] == board[i][1] && board[i][0] == board[i][2] && board[i][0] == bw) {
                return true;
            }

            if (board[0][i] == board[1][i] && board[0][i] == board[2][i] && board[0][i] == bw) {
                return true;
            }
        }

        if (board[0][0] == board[1][1] && board[0][0] == board[2][2] && board[0][0] == bw) {
            return true;
        }

        if (board[0][2] == board[1][1] && board[0][2] == board[2][0] && board[0][2] == bw) {
            return true;
        }

        return false;
    }

    void place(int row, int col, int square) {
        board[row][col] = square;
    }

    string toString() {
        ostringstream oss;
        oss << " ABC\n";

        for (int i = 0; i < 3; i++) {
            oss << (i + 1);
            for (int j = 0; j < 3; j++) {
                if (board[i][j] == BLACK) {
                    oss << "X";
                } else if (board[i][j] == WHITE) {
                    oss << "O";
                } else {
                    oss << ".";
                }
            }
            oss << " ABC\n";
        }

        return oss.str();
    }
};

struct System {
    vector<Game*> allGames;
    vector<User*> allUsers;

public:
    vector<string> getAllUsers() {
        vector<string> result;
        for (User *u : allUsers) {
            result.push_back(u->username);
        }
        return result;
    }

    User* findUser(const string username) {
        for (User *u : allUsers) {
            if (u->username == username) {
                return u;
            }
        }
        return nullptr;
    }

    Game* findGame(const string id) {
        for (Game *g : allGames) {
            if (g->id == id) {
                return g;
            }
        }
        return nullptr;
    }
};

System sys;

void printHelp();

//read one line from input stream.
string readLine(int socketId) {
    char ch;
    string s;
    while (read(socketId, &ch, 1)) {
        if (ch == '\n') {
            break;
        }
        s += ch;
    }
    return s;
}

//read a single word from input stream.
string readWord(int socketId) {
    char ch;
    string s;
    while (read(socketId, &ch, 1)) {
        if (ch == ' ') {
            break;
        }
        s += ch;
    }
    return s;
}

void writeLine(int socketId, const string line) {
    string temp = line + '\n';
    write(socketId, temp.c_str(), temp.size());
}

bool isMoveCommand(const string command) {
    if (command.size() == 2) {
        char ch1 = command[0];
        char ch2 = command[1];
        return (ch1 == 'A' || ch1 == 'B' || ch1 == 'C') && (ch2 == '1' || ch2 == '2' || ch2 == '3');
    }
    return false;
}

void threadFunc(int socketId) {
    User *user = nullptr;

    printf("client connected\n");

    string line;
    while ((line = readLine(socketId)).size() > 0) {
        string command;
        vector<string> parameters;

        //parse the line string from client
        istringstream iss(line);
        iss >> command;
        string temp;
        while (iss >> temp) {
            parameters.push_back(temp);
        }

        if (command == "register" && parameters.size() == 2) {
            if (user == nullptr) {

                string username = parameters[0];
                string password = parameters[1];
                user = sys.findUser(username);
                if (user == nullptr) {
                    user = new User(username, password);
                    user->login = true;
                    sys.allUsers.push_back(user);
                    writeLine(socketId, "Successfully logged into the system");
                    user->login = true;
                    user->sockId = socketId;
                } else {
                    if (user->login) {
                        user = nullptr;
                        writeLine(socketId, "The user already logged in");
                    } else if (user->password == password) {
                        writeLine(socketId, "Successfully logged into the system");
                        user->login = true;
                        user->sockId = socketId;
                    } else {
                        user = nullptr;
                        writeLine(socketId, "Incorrect password");
                    }
                }
            } else {
                writeLine(socketId, "You are already logged in");
            }
        } else if (command == "login" && parameters.size() == 2) {
            if (user == nullptr) {
                string username = parameters[0];
                string password = parameters[1];
                user = sys.findUser(username);
                if (user == nullptr) {
                    writeLine(socketId, "Incorrect username");
                } else {
                    if (user->login) {
                        user = nullptr;
                        writeLine(socketId, "The user already logged in");
                    } else if (user->password == password) {
                        writeLine(socketId, "Successfully logged into the system");
                        user->login = true;
                        user->sockId = socketId;
                    } else {
                        user = nullptr;
                        writeLine(socketId, "Incorrect password");
                    }
                }
            } else {
                writeLine(socketId, "You are already logged in");
            }
        } else if (command == "who" && parameters.size() == 0) {
            if (user == nullptr) {
                writeLine(socketId, "Please log in to the system first");
            } else {
                //List all online users
                vector<string> users = sys.getAllUsers();
                ostringstream oss;
                oss << users.size() << " users";
                writeLine(socketId, oss.str());
                for (string s : users) {
                    writeLine(socketId, s);
                }
            }
        } else if (command == "stats" && (parameters.size() == 0 || parameters.size() == 1)) {
            if (user == nullptr) {
                writeLine(socketId, "Please log in to the system first");
            } else {
                if (parameters.size() == 0) {
                    writeLine(socketId, user->information);
                } else {
                    User *u = sys.findUser(parameters[0]);
                    if (u == nullptr) {
                        writeLine(socketId, "The user not found");
                    } else {
                        writeLine(socketId, u->information);
                    }
                }
            }
        } else if (command == "game" && (parameters.size() == 0)) {
            if (user == nullptr) {
                writeLine(socketId, "Please log in to the system first");
            } else {
                //list all current games
                ostringstream oss;
                oss << sys.allGames.size() << " games";
                writeLine(socketId, oss.str());
                for (Game *g : sys.allGames) {
                    ostringstream oss2;
                    oss2 << g->id;
                    writeLine(socketId, oss2.str());
                }
            }
        } else if (command == "observe" && (parameters.size() == 1)) {
            if (user == nullptr) {
                writeLine(socketId, "Please log in to the system first");
            } else if (user->playId != "") {
                writeLine(socketId, "You are playing. Cannot observe a game");
            } else {
                //get game num
                Game *g = sys.findGame(parameters[0]);
                if (g == nullptr) {
                    writeLine(socketId, "Game not found");
                } else {
                    user->observeId = g->id;
                    writeLine(socketId, "You observe a game");
                }
            }
        } else if (command == "match" && (parameters.size() == 3 || parameters.size() == 4)) {
            if (user == nullptr) {
                writeLine(socketId, "Please log in to the system first");
            } else if (user->playId != "") {
                writeLine(socketId, "You are playing. Cannot start a game");
            } else {
                string username = parameters[1];
                User *other = sys.findUser(username);

                string bw = parameters[2];
                int t = 60;
                if (parameters.size() == 4) {
                    istringstream iss(parameters[3]);
                    t = -1;
                    iss >> t;
                }

                if ((bw != "b" && bw != "w") || t < 0) {
                    writeLine(socketId, "Invalid command format");
                } else if (t < 10) {
                    writeLine(socketId, "Time should at least 10 seconds");
                } else if (other == nullptr) {
                    writeLine(socketId, "Player not found");
                } else if (other->playId != "") {
                    writeLine(socketId, "The player is playing now");
                } else if (!other->login) {
                    writeLine(socketId, "The player is not online");
                } else if (other->isBlock(user->username)) {
                    writeLine(socketId, "The player blocked you");
                } else if (user->isBlock(other->username)) {
                    writeLine(socketId, "Your blocked the player");
                } else {
                    ostringstream oss1;
                    oss1 << user->username << " is inviting your a game";
                    writeLine(other->sockId, oss1.str());

                    string blackName = user->username;
                    string whiteName = other->username;
                    if (bw == "w") {
                        blackName = other->username;
                        whiteName = user->username;
                    }
                    Game *g = new Game(sys.allGames.size(), blackName, whiteName, t);
                    sys.allGames.push_back(g);

                    user->playId = g->id;
                    user->observeId = "";
                    other->playId = g->id;
                    other->observeId = "";

                    //start playing
                    User *blackUser = sys.findUser(blackName);
                    writeLine(blackUser->sockId, "You start a new game as black(X), please move");
                    User *whiteUser = sys.findUser(whiteName);
                    writeLine(whiteUser->sockId, "You start a new game as white(X)");
                }
            }
        } else if (isMoveCommand(command) && (parameters.size() == 0)) {
            if (user == nullptr) {
                writeLine(socketId, "Please log in to the system first");
            } else if (user->playId == "") {
                writeLine(socketId, "You are not playing");
            } else {
                Game *g = sys.findGame(user->playId);
                int row = command[1] - '1';
                int col = command[0] - 'A';
                if (g->board[row][col] != 0) {
                    writeLine(socketId, "The cell is not empty");
                } else {
                    int color;
                    if (user->username == g->blackUsername) {
                        g->place(row, col, BLACK);
                        color = BLACK;
                    } else {
                        g->place(row, col, WHITE);
                        color = WHITE;
                    }
                    if (g->turn == BLACK) {
                        g->turn = WHITE;
                    } else {
                        g->turn = BLACK;
                    }

                    writeLine(socketId, g->toString());
                    if (g->isWin(color)) {
                        writeLine(socketId, "You win");

                        if (user->username == g->blackUsername) {
                            User *u = sys.findUser(g->whiteUsername);
                            writeLine(u->sockId, "You lose");
                        } else {
                            User *u = sys.findUser(g->blackUsername);
                            writeLine(u->sockId, "You lose");
                        }
                    }
                }
            }

        } else {
            writeLine(socketId, "unknown command");
        }
    }
    close(socketId);

    if (user != nullptr) {
        user->login = false;

//check if it's playing.

    }

    printf("client disconnected\n");
}

void startServer() {
    int serverSocket;
    struct sockaddr_in serverAddr;
    struct sockaddr_in clientAddr;

    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("failed to create server socket\n");
        return;
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(serverSocket, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) < 0) {
        perror("failed to bind server socket\n");
        return;
    }

    if (listen(serverSocket, 10) < 0) {
        perror("failed to listen\n");
        return;
    }

//accept clients and create thread for each connection.
    while (true) {
        int clientSocket;
        int addrLen = sizeof(clientAddr);

        clientSocket = accept(serverSocket, (struct sockaddr*) &clientAddr, (socklen_t*) &addrLen);
        if (clientSocket < 0) {
            perror("failed to accept\n");
        } else {
            int pid = fork();
            if (pid == 0) {
                threadFunc(clientSocket);
                exit(0);
            }
        }
    }

    close(serverSocket);
}

int main(int argc, char *argv[]) {
    startServer();

    return 0;
}

void printHelp() {
    printf("who\n");
    printf("stats [name]\n");
    printf("game\n");
    printf("observe <game_num>\n");
    printf("unobserve\n");
    printf("match <name> <b|w> [t]\n");
    printf("<A|B|C><1|2|3>\n");
    printf("resign\n");
    printf("refresh\n");
    printf("shout <msg>\n");
    printf("tell <name> <msg>\n");
    printf("kibitz <msg>\n");
    printf("' <msg>\n");
    printf("quiet\n");
    printf("nonquiet\n");
    printf("block <id>\n");
    printf("unblock <id>\n");
    printf("listmail\n");
    printf("readmail <msg_num>\n");
    printf("deletemail <msg_num>\n");
    printf("mail <id> <title>\n");
    printf("info <msg>\n");
    printf("passwd <new>\n");
    printf("exit\n");
    printf("quit\n");
    printf("help\n");
    printf("?\n");
    printf("register <name> <pwd>\n");
}
