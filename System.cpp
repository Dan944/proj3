#include "System.h"
#include "User.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <chrono>
#include <algorithm>
#include <cctype>
#include <vector>
#include <string>

using namespace std;
time_t stringToTimeT(const std::string& timeString) {
    struct tm tm;
    memset(&tm, 0, sizeof(struct tm));

    // Parse the date string
    if (strptime(timeString.c_str(), "%a %b %d %H:%M:%S %Y", &tm) == nullptr) {
        cerr <<"string can not be processed:"<< timeString<<endl;
        cerr << "Failed to parse date" << endl;
        return static_cast<time_t>(-1); // Return an error value
    }

    // Convert tm structure to time_t
    time_t time = mktime(&tm);
    if (time == -1) {
        cerr << "Failed to convert to time_t" << endl;
    }
    
    return time;
}

// Gets all usernames from the system's user list
std::vector<std::string> System::getAllUsers() {
    std::vector<std::string> usernames;
    for (auto& user : allUsers) {
        usernames.push_back(user->username);
    }
    return usernames;
}

void System::rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// Initializes the system, possibly by loading user data
void System::init() {
    load_user();
    load_mail();
}

void System::writeLine(int socketId, const std::string line) {
    std::string temp = line + '\n';
    write(socketId, temp.c_str(), temp.size());
}

// Displays information about who is currently online, placeholder implementation
void System::who(int fd) {
    onlineUpdate();
    char out1[100];
    int usern = onlineUsers.size();
    sprintf(out1,"Total %d user(s) online:\n", usern);
    std::string out2;
    for (User* u : onlineUsers) {
        out2 += u->username;
        out2 += " ";
    }
    write(fd, out1, strlen(out1));
    writeLine(fd, out2);
}

void System::regist(int fd, std::string username, std::string password) {
    User *user1 = findUser(username);
    if (user1 != nullptr ){
        writeLine(fd, "This name has been registed, please user another name");
        return;
    }
    rtrim(password);
    User *user = new User(username, password);
    for (int i=1; i<=1024; i++){
        bool ava = true;
        for (User *u : allUsers) {
            if (u->id == i) {
                ava = false;
            }
        }
        if (ava) {
            user->id = i;
            break;
        }
    }
    allUsers.push_back(user);
    System::saveUserData();
    writeLine(fd, "registed sucessful");
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
        User *user = new User();
        std::stringstream ss(line);
        std::string item;
        std::vector<std::string> tokens;
        while (std::getline(ss, item, ',')) {
            tokens.push_back(item);
        }
        tokens.push_back("");
        user->setUsername(tokens[0]);
        user->setPassword(tokens[1]);
        user->setId(std::stoi(tokens[2]));
        user->setInformation(tokens[3]);
        user->setRating(std::stof(tokens[4]));
        user->setWin(std::stoi(tokens[5]));
        user->setLoss(std::stoi(tokens[6]));
        user->setQuiet(std::stoi(tokens[7]));
        std::istringstream blockStream(tokens[8]); // Use tokens[8] to create a stream
        std::string blockName;
        while (std::getline(blockStream, blockName, ';')) {
            user->blocked_names.push_back(blockName);
        }
        allUsers.push_back(user);
    }
    file.close();
}

User* System::findUser(std::string username) {
    rtrim(username);
    for (User* u : allUsers) {
        if (u->getUsername() == username) {
            return u;
        }
    }
    return nullptr;
}

User* System::findUserFd(const int fd) {
    for (User *u : allUsers) {
        if (u->sockId == fd) {
            return u;
        }
    }
    return nullptr;
}

void System::onlineUpdate(){
    onlineUsers.clear();
    for (User* u : allUsers) {
        if (u->login) { // If the user is marked as online
            onlineUsers.push_back(u);
        }
    }
}

void System::printAllUsers() const {
    std::cout << "All users:\n";
    for (const User* user : allUsers) {
        std::cout << user->getUsername() << std::endl; // Assuming User class has a getUsername() method
    }
}


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
                << blockedNamesStream.str() <<std::endl;
    }

    outFile.close();
    std::cout << "User data saved successfully." << std::endl;
}

void System::stats(int fd, const std::string name){
    char statsStr[1024];
    User *user = System::findUser(name);
    if (user == nullptr){
        writeLine(fd,"User does not exist.");
        return;
    }
    int i=0;
    const char* boolStr1 = user->quiet ? "Yes" : "No";
    const char* boolStr2 = user->login ? "Online" : "Offline";
    std::string blockStr = "";
    char loginStr[100] = "";
    sprintf(loginStr,"%s is currently %s.",user->username.c_str(),boolStr2);
    int usern = user->blocked_names.size();
    for ( i = 0; i < usern; i++) {
        blockStr += user->blocked_names[i];
        blockStr += " ";
    }
    sprintf(statsStr,"User: %s\nInfo: %s\nRating: %f\nWins: %d, Loses: %d\nQuiet: %s\nBlocked users: %s\n\n%s\n",
        user->username.c_str(),user->information.c_str(),user->rating,user->win,user->loss,boolStr1,blockStr.c_str(),loginStr);
    write(fd,statsStr,strlen(statsStr));
}

void System::help(int fd){
    std::string helps = 
        "Commands supported:\n"
        "  who                     # List all online users\n"
        "  stats [name]            # Display user information\n"
        "  game                    # list all current games\n"
        "  observe <game_num>      # Observe a game\n"
        "  unobserve               # Unobserve a game\n"
        "  match <name> <b|w> [t]  # Try to start a game\n"
        "  <A|B|C><1|2|3>          # Make a move in a game\n"
        "  resign                  # Resign a game\n"
        "  refresh                 # Refresh a game\n"
        "  shout <msg>             # shout <msg> to every one online\n"
        "  tell <name> <msg>       # tell user <name> message\n"
        "  kibitz <msg>            # Comment on a game when observing\n"
        "  ' <msg>                 # Comment on a game\n"
        "  quiet                   # Quiet mode, no broadcast messages\n"
        "  nonquiet                # Non-quiet mode\n"
        "  block <name>            # No more communication from <name>\n"
        "  unblock <name>          # Allow communication from <name>\n"
        "  listmail                # List the header of the mails\n"
        "  readmail <msg_num>      # Read the particular mail\n"
        "  deletemail <msg_num>    # Delete the particular mail\n"
        "  mail <id> <title>       # Send id a mail\n"
        "  info <msg>              # change your information to <msg>\n"
        "  passwd <new>            # change password\n"
        "  exit                    # quit the system\n"
        "  quit                    # quit the system\n"
        "  help                    # print this message\n"
        "  ?                       # print this message";
    writeLine(fd, helps);
}
void System::info(int fd, char* buf){
    char *token = strchr(buf, ' '); 
    if (token != NULL) {
        User *user = findUserFd(fd);
        token++;
        char *msg = strchr(token, '\n');
        if (msg != NULL) {
            *msg = '\0';
        }
        user->information = token;
        saveUserData();
    }
    else {
        writeLine(fd,"Please enter information as info <msg>");
    }
}

void System::shout(int fd, char* buf) {
    char *token = strchr(buf, ' '); 
    if (token != NULL) {
        token++;
        char *end = strchr(token, '\n');
        if (end != NULL) {
            *end = '\0';
        }
        onlineUpdate();
        char msg[100];
        User *user = findUserFd(fd);
        sprintf(msg,"!shout! *%s*: %s", user->username.c_str(), token);
        for (User* u : onlineUsers) {
            if (u->quiet==false) {
                writeLine(u->sockId,string(msg));
            }
        }
    }
    else {
        writeLine(fd,"Please enter information as shout <msg>");
    }
}
void System::passwd(int fd, char* buf){
    User *user = findUserFd(fd);
    char* token = strtok(buf, " ");
    token = strtok(NULL, " ");
    if (token != NULL) {
        string pwd = token;
        rtrim(pwd);
        user->password = pwd;
        writeLine(fd,"password changed");
        saveUserData();
    }
    else {
        writeLine(fd,"Please enter information as passwd <pwd>");
    }
}
void System::block(int fd, char* buf){
    User *user = findUserFd(fd);
    char* token = strtok(buf, " ");
    token = strtok(NULL, " ");
    if (token != NULL) {
        string name = token;
        rtrim(name);
        User* buser = findUser(name);
        if (buser == nullptr) {
            writeLine(fd, "No user " + name);
        }
        else if (find(user->blocked_names.begin(), user->blocked_names.end(), name) != user->blocked_names.end()) {
            writeLine(fd, "User " + name + " has already been blocked");
        }
        else {
            user->blocked_names.push_back(buser->username);
            writeLine(fd, "User "+name+" blocked");
            saveUserData();
        }
    }
    else {
        writeLine(fd,"Please enter information as block <name>");
    }
}
void System::unblock(int fd, char* buf){
    User *user = findUserFd(fd);
    char* token = strtok(buf, " ");
    token = strtok(NULL, " ");
    if (token != NULL) {
        string name = token;
        rtrim(name);
        if (find(user->blocked_names.begin(), user->blocked_names.end(), name) != user->blocked_names.end()) {
            user->blocked_names.erase(remove(user->blocked_names.begin(), user->blocked_names.end(), name), 
                user->blocked_names.end());
            writeLine(fd, "Unblocked "+name);
            saveUserData();
        } else {
            writeLine(fd, "User " +name+ " was not blocked");
        }
    }
    else {
        writeLine(fd,"Please enter information as unblock <name>");
    }
}
void System::tell(int fd, char* buf){
    const char* delim = " ";
    strtok(buf, delim);
    char* name = strtok(NULL, delim);
    if (name == NULL) {
        writeLine(fd, "Please enter information as tell <name> <msg>");
        return;
    }
    User* duser = findUser(string(name));
    if (duser==nullptr || duser->login==false) {
        writeLine(fd, "No such online user " + string(name));
        return;
    }
    User* user = findUserFd(fd);
    if (find(duser->blocked_names.begin(), duser->blocked_names.end(), user->username) != duser->blocked_names.end()) {
        writeLine(fd, "You have been blocked");
        return;
    }
    char* msg = name + strlen(name) + 1;
    string outmsg = string(msg);
    rtrim(outmsg);
    writeLine(duser->sockId, outmsg);
}
int System::send_mail_1(int fd, char* buf){
    const char* delim = " ";
    strtok(buf, delim);
    char* name = strtok(NULL, delim);
    if (name == NULL) {
        writeLine(fd, "Please enter information as mail <name> <tittle>");
        return 0;
    }
    User* duser = findUser(string(name));
    if (duser==nullptr) {
        writeLine(fd, "No such user " + string(name));
        return 0;
    }
    User* user = findUserFd(fd);
    if (find(duser->blocked_names.begin(), duser->blocked_names.end(), user->username) != duser->blocked_names.end()) {
        writeLine(fd, "You have been blocked");
        return 0;
    }
    char* msg = name + strlen(name) + 1;
    string tittle = string(msg);
    rtrim(tittle);
    if(tittle.length()==0){
        tittle = "No tittle";
    }
    Email *email = new Email();
    email->tittle = tittle;
    email->send_name = user->username;
    email->rec_name = duser->username;
    user->current_email=email;
    writeLine(user->sockId, "Please input mail body, finishing with '.' at the beginning of a line");

    return 1;
}

int System::send_mail_2(int fd, char* buf){
    User* user = findUserFd(fd);
    if (strncmp(buf,".",1)==0) {
        user->current_email->send_time = time(0);
        User* duser = findUser(user->current_email->rec_name);
        duser->emails.push_back(user->current_email);
        writeLine(fd, "Send successfully.");
        // writeLine(fd, "tittle:");
        // writeLine(fd, user->current_email->tittle);
        // writeLine(fd, "time:");
        // time_t otime = user->current_email->send_time;
        // writeLine(fd, string(ctime(&otime)));
        // writeLine(fd, "body:");
        // writeLine(fd, user->current_email->content);

        if (duser->login == true) {
            writeLine(duser->sockId,"You have received a new email");
        }
        saveMailData();
        return 1;
    }
    else {
        user->current_email->content += string(buf);
        return 0;
    }
}

void System::list_mail(int fd, char* buf){
    User* user = findUserFd(fd);
    if (user->emails.size()==0) {
        writeLine(fd, "You have no messages.");
    }
    else {
        writeLine(fd, "Your messages:");
        int count = 0;
        for (auto& email : user->emails) {
            const char* read = email->read ? "Read" : "New";
            char line[1024];
            sprintf(line, "  %i  %s  %s  '%s'  %s",count,read,email->send_name.c_str(),email->tittle.c_str(),
                ctime(&(email->send_time)));
            write(fd, line, strlen(line));
            count++;
        }
    }
}

void System::read_mail(int fd, char* buf){
    User *user = findUserFd(fd);
    char* token = strtok(buf, " ");
    token = strtok(NULL, " ");
    if (user->emails.size()==0) {
        writeLine(fd, "You have no messages.");
        return;
    }
    if (token != NULL) {
        int mail_id = stoi(token);
        if (mail_id > int(user->emails.size())){
            writeLine(fd, "Message number invalid");
            return;
        }
        Email *email = user->emails[mail_id];
        char mail_str[2048];
        sprintf(mail_str,"From: %s\nTitle:  %s\nTime: %s\n%s\n",email->send_name.c_str(),email->tittle.c_str(),
            ctime(&(email->send_time)),email->content.c_str());
        writeLine(fd, string(mail_str));
        user->emails[mail_id]->read=true;
        saveMailData();
        return;
    }
    else {
        writeLine(fd,"Please enter information as readmail <mail id>");
        return;
    }
}

void System::delete_mail(int fd, char* buf){
    User *user = findUserFd(fd);
    char* token = strtok(buf, " ");
    token = strtok(NULL, " ");
    if (user->emails.size()==0) {
        writeLine(fd, "You have no messages.");
        return;
    }
    if (token != NULL) {
        int mail_id = stoi(token);
        if (mail_id > int(user->emails.size())){
            writeLine(fd, "Message number invalid");
            return;
        }
        user->emails.erase(user->emails.begin()+mail_id);
        writeLine(fd, "sucessfully delete");
        saveMailData();
        return;
    }
    else {
        writeLine(fd,"Please enter information as deletemail <mail id>");
        return;
    }
}
void System::saveMailData(){
    string rootPath = "data/mails/";
    for (auto& user : allUsers) {
        if (user->emails.size()==0) {
            continue;
        }
        string filePath=rootPath+user->username;
        ofstream outFile(filePath, ofstream::trunc);
        if (!outFile.is_open()) {
            std::cerr << "Failed to open file for writing: " << filePath << endl;
            continue;
        }
        for (auto& email : user->emails){
            rtrim(email->content);
            outFile << email->tittle << endl
                    << email->send_name << endl
                    << email->rec_name << endl
                    << string(ctime(&(email->send_time)))
                    << string(email->read ? "Read" : "New") << endl
                    << email->content <<endl<< "." <<endl;
        }

        outFile.close();
        cout << "Email data saved successfully." << endl;
        }
}
void System::load_mail(){
    string rootPath = "data/mails/";
    for (auto& user : allUsers) {
        string filePath=rootPath+user->username;
        ifstream file(filePath);
        if (!file.is_open()) {
            continue;
        }
        string line;
        int count = 0;
        Email *email = new Email();
        while (getline(file, line)){
            if (count == 0) { email->tittle = line;}
            else if (count == 1) { email->send_name = line;}
            else if (count == 2) { email->rec_name = line;}
            else if (count == 3) { email->send_time = stringToTimeT(line);}
            else if (count == 4) { istringstream(line)>>boolalpha>>email->read;}
            else if (line == ".") { 
                count=0;
                Email *mail = new Email();
                mail->tittle = email->tittle;
                mail->send_name = email->send_name;
                mail->rec_name = email->rec_name;
                mail->send_time = email->send_time;
                mail->read = email->read;
                mail->content = email->content;
                user->emails.push_back(mail);
                continue;
            }
            else {
                email->content += line;
            }
            count++;
        }
        file.close();
    }
}
