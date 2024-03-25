#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <cstring>
#include <fstream>
#include <sstream>
#include <locale>
#include <cctype>

using namespace std;
vector<int> states(100, 0);
// char* guest_username[100];
// char* guest_password[100];
vector<string> guest_username(100);
vector<string> guest_password(100);

void sig_chld(int signo)
{
	pid_t pid;
	int stat;
	while ((pid = waitpid(-1, &stat, WNOHANG)) > 0) 
		printf("child %d terminated.\n", pid);
	return ;
}

void print_fd_set(const fd_set *set, int ccc) {
    for (int i = 0; i < ccc; ++i) {
        if (FD_ISSET(i, set)) {
            printf("FD %d is set\n", i);
        }
    }
}

static inline void rtrim(string &s) {
    s.erase(find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !isspace(ch);
    }).base(), s.end());
}

void writeLine(int socketId, const string line) {
    string temp = line + '\n';
    write(socketId, temp.c_str(), temp.size());
}

struct User {
    string username; 
    string password;
    string information; 
    vector<string> blocked_names;
    bool quiet; /* whether this user is quiet*/
    bool login; /* whether this user is online*/
    string observeId;
    string playId;
    int sockId;
	float rating;
	int win;
	int loss;
	int id;
	int cmd;

    User(string username, string password) {
        this->username = username;
        this->password = password;
        information = username;
        quiet = false;
        login = false;
        sockId = -1;
		cmd = 0 ;
		win = 0;
		loss = 0;
		rating = 0;

    }
	User() {
        this->username = username;
        this->password = password;
        information = username;
        quiet = false;
        login = false;
        sockId = -1;
		cmd = 0;
		win = 0;
		loss = 0;
		rating = 0;
    }
	void logout() {
		login = false;
		cmd = 0;
		sockId = -1;
		//game-related should be dealed here
	}
	void writef(string buf) {
		char outbuf[1024];
		sprintf(outbuf, "<%s: %i> %s", username.c_str(), cmd, buf.c_str());
		cmd++;
		// Ensure num is the length of outbuf, not the result of the read call
		write(sockId, outbuf, strlen(outbuf)); 
	}
};
struct System {
    vector<User*> allUsers;
	vector<User*> onlineUsers;
	vector<string> getAllUsers() {
        vector<string> result;
        for (User *u : allUsers) {
            result.push_back(u->username);
        }
        return result;
    }
	User* findUser(string username) {
		rtrim(username);
        for (User *u : allUsers) {
            if (u->username == username) {
				cout << "finded\n";
                return u;
            }
        }
        return nullptr;
    }
	User* findUserFd(const int fd) {
        for (User *u : allUsers) {
            if (u->sockId == fd) {
                return u;
            }
        }
        return nullptr;
    }
	void init() {
		System::load_user();
	}
	void onlineUpdate(){
		onlineUsers.clear();
		for (User* u : allUsers) {
			if (u->login) { // If the user is marked as online
				onlineUsers.push_back(u);
			}
		}
	}
	void who(int fd) {
		onlineUpdate();
		char out1[100];
		sprintf(out1,"Total %i user(s) online:\n", onlineUsers.size());
		string out2;
		for (User* u : onlineUsers) {
			out2 += u->username;
			out2 += " ";
		}
		write(fd, out1, strlen(out1));
		writeLine(fd, out2);
	}
	void load_user() {
		string filePath = "data/user"; // Adjust the path as necessary
		ifstream file(filePath);
		if (!file.is_open()) {
			cerr << "Failed to open user file." << endl;
		}
		string line;
		getline(file, line);
		while (getline(file, line)) {
			// user,password,id,info,rating,win,loses,quiet,block,online
			int count = 0;
			User *user = new User();
			stringstream ss(line);
			string item;
			vector<string> tokens;
			while (getline(ss, item, ',')) {
				tokens.push_back(item);
			}
			tokens.push_back("");
			user->username = tokens[0];
			user->password = tokens[1];
			user->id = stoi(tokens[2]);
			user->information = tokens[3];
			user->rating = stof(tokens[4]);
			user->win = stoi(tokens[5]);
			user->loss = stoi(tokens[6]);
			user->quiet = stoi(tokens[7]);
			istringstream blockStream(tokens[8]); // Use tokens[8] to create a stream
			string blockName;
			while (getline(blockStream, blockName, ';')) {
				user->blocked_names.push_back(blockName);
			}
			allUsers.push_back(user);
		}
		file.close();
	}
	void regist(int fd, string username, string password) {
		User *user1 = findUser(username);
		if (user1 != nullptr ){
			writeLine(fd, "This name has been registed, please user another name");
			return;
		}
		rtrim(password);
		User *user = new User(username, password);
		for (int i=0; i<=1024; i++){
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
		System::save();
		writeLine(fd, "registed sucessful");
	}
	void save(){
		string filePath = "data/user";
		ofstream outFile(filePath, ofstream::trunc); // Open the file in truncate mode to overwrite

		if (!outFile.is_open()) {
			cerr << "Failed to open file for writing: " << filePath << endl;
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
				<< "BlockName" << endl;

		for (const User* user : allUsers) {
			// Convert blocked_ids vector to a string
			printf("loop\n");
			stringstream blockedNamesStream;
			for (size_t i = 0; i < user->blocked_names.size(); ++i) {
				blockedNamesStream << user->blocked_names[i];
				if (i < user->blocked_names.size() - 1) blockedNamesStream << ";"; // delimiter for blocked_ids
			}

			// Writing user data, converting boolean to string for clarity
			outFile << user->username << ","
					<< user->password << ","
					<< user->id << ","
					<< user->information << ","
					<< user->rating << ","
					<< user->win << ","
					<< user->loss << ","
					<< (user->quiet ? true : false) << ","
					<< blockedNamesStream.str() << endl;
		}

		outFile.close();
		cout << "User data saved successfully." << endl;
	}
	void stats(int fd, string name){
		char statsStr[1024];
		User *user = findUser(name);
		if (user == nullptr){
			writeLine(fd,"User does not exist.");
			return;
		}
		int i=0;
		const char* boolStr1 = user->quiet ? "Yes" : "No";
		const char* boolStr2 = user->login ? "Online" : "Offline";
		string blockStr = "";
		char loginStr[100] = "";
		sprintf(loginStr,"%s is currently %s.",user->username.c_str(),boolStr2);
		for ( i = 0; i < user->blocked_names.size(); i++) {
			blockStr += user->blocked_names[i];
			blockStr += " ";
		}
		sprintf(statsStr,"User: %s\nInfo: %s\nRating: %f\nWins: %d, Loses: %d\nQuiet: %s\nBlocked users: %s\n\n%s\n",
			user->username.c_str(),user->information.c_str(),user->rating,user->win,user->loss,boolStr1,blockStr.c_str(),loginStr);
		write(fd,statsStr,strlen(statsStr));
	}
	void help(int fd){
		string helps = 
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
			"  block <id>              # No more communication from <id>\n"
			"  unblock <id>            # Allow communication from <id>\n"
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
	void info(int fd, char* buf){
		char *token = strchr(buf, ' '); 
		int argCount = 0;
		if (token != NULL) {
			User *user = findUserFd(fd);
			token++;
			char *msg = strchr(token, '\n');
			if (msg != NULL) {
				*msg = '\0';
			}
			user->information = token;
		}
		else {
			writeLine(fd,"Please enter information as info <msg>");
		}
	}
};
System sys;


void login(int rec_sock, string username, string password) {
	char buf[1024] = "username(guest):";
	rtrim(password);
	// string dusername = username.substr(0, username.length() - 2);
	// string dpassword = password.substr(0, password.length() - 2);
	User *user = sys.findUser(username);
	if (user == nullptr) {
		writeLine(rec_sock, "Incorrect username");
		states[rec_sock] = 0;
		write(rec_sock, buf, strlen(buf));
	} else if (user->password==password){
		if (user->login==true) {
			states[user->sockId]=-1;
			char q[100];
			strcpy(q,"sorry, your account has been login in other place, enter any button to quit\n");
			write(user->sockId,q,strlen(q));
			user->sockId = rec_sock;
			states[rec_sock] = 3;
			user->cmd = 0;
		} else {
			user->login = true;
			user->sockId = rec_sock;
			states[rec_sock] = 3;
		}
	} else {
		writeLine(rec_sock, "Incorrect password");
		cout << "user.password = "<<user->password<<endl;
		cout << "password = "<<password<<endl;
		states[rec_sock] = 0;
		write(rec_sock, buf, strlen(buf));
	}
}

void print_hello(int rec_sock) {
    // Make the buffer static so its lifetime extends beyond the function call.
    // Note: This makes the function non-reentrant and not thread-safe.
    char hello[1024];
	char buf[1024];
    sprintf(hello, 
        "%s%s%s%s%s",
        "********************************************************************\n",
        "You are attempting to log into online tic-tac-toe Server.\n",
        "Please be advised by continuing that you agree to the terms of the\n",
        "Computer Access and Usage Policy of online tic-tac-toe Server.\n\n",
        "********************************************************************\n\n\n");
	if (write(rec_sock, hello, strlen(hello)) < 0) {
		perror("hello error");
	}
	strcpy(buf,"username(guest):");
	write(rec_sock, buf, strlen(buf));
}

void start_server(char* port) {
	int sockfd, rec_sock;
	socklen_t len;
	vector<int> sock_vector;
	struct sockaddr_in addr, recaddr;
	struct sigaction abc;
	char buf[100];
	char outbuf[100];
	fd_set allset, rset;
	int maxfd;

	abc.sa_handler = sig_chld;
	sigemptyset(&abc.sa_mask);
	abc.sa_flags = 0;

	sigaction(SIGCHLD, &abc, NULL);

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror(": Can't get socket");
		exit(1);
	}

	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_family = AF_INET;
	addr.sin_port = htons((short)atoi(port));

	if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror(": bind");
		exit(1);
	}

	len = sizeof(addr);
	if (getsockname(sockfd, (struct sockaddr *)&addr, &len) < 0) {
		perror(": can't get name");
		_exit(1);
	}

	if (listen(sockfd, 5) < 0) {
		perror(": bind");
		exit(1);
	}


	FD_ZERO(&allset);
	FD_SET(sockfd, &allset);
	maxfd = sockfd;
	sock_vector.clear();

	while (1) {
		rset = allset;
		select(maxfd+1, &rset, NULL, NULL, NULL);
		// printf("FD_ISSET[%d]\n",FD_ISSET(sockfd, &rset));
		if (FD_ISSET(sockfd, &rset)) {
			/* somebody tries to connect */
			// printf("I am ISSET before accept\n");
			// printf("before ip = %s, port = %d\n", inet_ntoa(recaddr.sin_addr), htons(recaddr.sin_port));
			if ((rec_sock = accept(sockfd, (struct sockaddr *)(&recaddr), &len)) < 0) {
				if (errno == EINTR)
					continue;
				else {
					perror(":accept error");
					exit(1);
				}
			}
			print_hello(rec_sock);
			sock_vector.push_back(rec_sock);
			FD_SET(rec_sock, &allset);
			if (rec_sock > maxfd) maxfd = rec_sock;
		}

		auto itr = sock_vector.begin(); 
		while (itr != sock_vector.end()) {
			int num, fd;
			fd = *itr;
			if (FD_ISSET(fd, &rset)) {
				memset(buf, 0, sizeof(buf));
				num = read(fd, buf, 100);
				printf("current fd:[%i]\n",fd);
				if (num == 0) {
					/* client exits */
					printf("dandandadandnandandanadnadnadnadadnand\n");
					close(fd);
					FD_CLR(fd, &allset);
					User *user = sys.findUserFd(fd);
					if (user != nullptr) {
						user->logout();
					}
					states[fd] = 0;
					itr = sock_vector.erase(itr);
					continue;
				} //quit
				if (states[fd]==-1 || strncmp(buf, "quit", 4) == 0 || strncmp(buf, "exit", 4) == 0) {
					// printf("gogoin: %s\n",buf);
					// printf("Client sent 'quit'. Closing connection.\n");
					printf("close socket [%d]\n",fd);
					writeLine(fd, "Man, see you again!");
					close(fd);
					FD_CLR(fd, &allset);
					User *user = sys.findUserFd(fd);
					if (user != nullptr) {
						user->logout();
					}
					states[fd] = 0;
					itr = sock_vector.erase(itr);
					continue;
				} // login-username
				else if (states[fd] == 0) {
					if (strncmp(buf,"guest",5) == 0){
						writeLine(fd, "You can only use 'register username password' as a guest.");
						states[fd] = 2;
					}
					else {
						guest_username[fd] = buf;
						write(fd, "password:", strlen("password:"));
						states[fd] = 1;
					}
				} //login-password 
				else if (states[fd] == 1) {
					guest_password[fd] = buf;
					login(fd,guest_username[fd],guest_password[fd]);
					// states[fd]=3;
				} //register
				else if (states[fd] == 2) {
					if (strncmp(buf,"register",8) != 0) {
						writeLine(fd, "You are not supposed to do this.\nYou can only use 'register username password' as a guest.");
						continue;
					}
					else {
						char *token = strtok(buf, " "); 
						token = strtok(NULL, " ");
						if (token == NULL) {
							writeLine(fd, "You are not supposed to do this.\nYou can only use 'register username password' as a guest.");
							continue;
						}
						else {
							string username = token;
							token = strtok(NULL, " ");
							if (token == NULL) {
								writeLine(fd, "You are not supposed to do this.\nYou can only use 'register username password' as a guest.");
								continue;
							}
							else {
								string password = token;
								sys.regist(fd, username,password);
							}
						}
					}

				} //who
				else if (states[fd] >= 3 && strncmp(buf, "who", 3) == 0) {
					sys.who(fd);
				} //stats
				else if (states[fd] >= 3 && strncmp(buf,"stats",5) == 0) {
					char *token = strtok(buf, " "); 
					int argCount = 0;
					token = strtok(NULL, " ");
					if (token != NULL) {
						sys.stats(fd, token);
					}
					else {
						User *user = sys.findUserFd(fd);
						sys.stats(fd, user->username);
					}
				}
				else if (states[fd] >= 3 && (strncmp(buf, "help", 4) == 0) || (strncmp(buf, "?", 1) == 0)) {
					sys.help(fd);
				}
				else if (states[fd] >= 3 && (strncmp(buf, "info", 4) == 0)) {
					sys.info(fd, buf);
				}
				if (states[fd]>=3){
					User *user = sys.findUserFd(fd);
					user->writef("");
				} 
			}
			++itr;
		}

		maxfd = sockfd;
		if (!sock_vector.empty()) {
			maxfd = max(maxfd, *max_element(sock_vector.begin(),
						sock_vector.end()));
		}
	}
}

int main(int argc, char * argv[])
{
	if (argc < 2) {
		printf("Usage: a.out port.\n");
		exit(0);
	}
	sys.init();
	start_server(argv[1]);
	// int a;
	// printf("%i\n",a);
}



