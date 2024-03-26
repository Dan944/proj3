#ifndef USER_H
#define USER_H

#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include <cstring>
#include <locale>
#include <cctype>
class User {

public:
    std::string username; 
    std::string password;
    int id;
    int sockId;
    std::string information; 
    bool quiet;
    bool login; // whether this user is online
    std::string observeId;
    std::string playId;
	float rating;
	int win;
	int loss;
    int cmd;
    std::vector<std::string> blocked_names;
    // : username(username), 
    //   password(password),
    //   id(id), // Assuming default id is 0 or an appropriate default value
    //   sockId(-1), // -1 often used to indicate an invalid socket id
    //   information(""),
    //   rating(0.0), // Assuming default rating is 0.0 or an appropriate default value
    //   quiet(false),
    //   login(false),
    //   observeId(""),
    //   playId(""),
    //   win(0),
    //   loss(0),
    //   cmd(0) 

    static std::vector<User*> allUsers;
    static User* findUser(const std::string& username);
    void logout();
    void writef(std::string buf);

    // Getters
    std::string getUsername() const;
    std::string getPassword() const;
    std::string getInformation() const;
    std::vector<std::string> getBlockedNames() const;
    bool isQuiet() const;
    bool isLogin() const;
    std::string getObserveId() const;
    std::string getPlayId() const;
    int getSockId() const;
    float getRating() const;
    int getWin() const;
    int getLoss() const;
    int getId() const;
    int getcmd() const;

    // Setters
    void setUsername(const std::string& val);
    void setPassword(const std::string& val);
    void setInformation(const std::string& val);
    void setBlockedNames(const std::vector<std::string>& val);
    void setQuiet(bool val);
    void setLogin(bool val);
    void setObserveId(const std::string& val);
    void setPlayId(const std::string& val);
    void setSockId(int val);
    void setRating(float val);
    void setWin(int val);
    void setLoss(int val);
    void setId(int val);
    void setcmd(int cmd);

    User(std::string username, std::string password);
	User();
    User(const User &other){
        username = other.username;
        password = other.password;
        id = other.id;
        sockId = other.sockId;
        information = other.information;
        quiet = other.quiet;
        login = other.login;
        observeId = other.observeId;
        playId = other.playId;
        rating = other.rating;
        win = other.win;
        loss = other.loss;
        cmd = other.cmd;
        blocked_names = other.blocked_names;
    }
};


#endif // USER_H