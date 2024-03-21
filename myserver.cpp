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

static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
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
    vector<int> blocked_ids;
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
    }
	User() {
        this->username = username;
        this->password = password;
        information = username;
        quiet = false;
        login = false;
        sockId = -1;
		cmd = 0;
    }
	void logout() {
		login = false;
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
	vector<string> getAllUsers() {
        vector<string> result;
        for (User *u : allUsers) {
            result.push_back(u->username);
        }
        return result;
    }
	User* findUser(string username) {
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
	void who() {	
	}
	void load_user() {
		std::string filePath = "data/user"; // Adjust the path as necessary
		std::ifstream file(filePath);
		if (!file.is_open()) {
			std::cerr << "Failed to open user file." << std::endl;
		}
		std::string line;
		std::getline(file, line);
		while (std::getline(file, line)) {
			// user,password,id,info,rating,win,loses,quiet,block,online
			int count = 0;
			User *user = new User();
			std::stringstream ss(line);
			std::string item;
			std::vector<std::string> tokens;
			while (std::getline(ss, item, ',')) {
				tokens.push_back(item);
			}
			user->username = tokens[0];
			user->password = tokens[1];
			user->id = std::stoi(tokens[2]);
			user->information = tokens[3];
			user->rating = std::stof(tokens[4]);
			user->win = std::stoi(tokens[5]);
			user->loss = std::stoi(tokens[6]);
			user->quiet = std::stoi(tokens[7]);		
			std::istringstream blockStream(tokens[8]); // Use tokens[8] to create a stream
			std::string blockId;
			while (std::getline(blockStream, blockId, ';')) {
				user->blocked_ids.push_back(std::stoi(blockId));
			}
			// cout << "*********************************************" <<endl;
			// cout << "username:" << user->username<<endl;
			// cout << "password:" << user->password<<endl;
			// cout << "id:" << user->id<<endl;
			// cout << "info:" << user->information<<endl;
			// cout << "rating:" << user->rating<<endl;
			// cout << "win:" << user->win<<endl;
			// cout << "loss:" << user->loss<<endl;
			// cout << "quiet:" << user->quiet<<endl;
			// cout << "block:";
			// for (const auto& num : user->blocked_ids) {
			// 	cout << num << ",";
			// }
			// cout << endl;
			allUsers.push_back(user);
		}
		file.close();
	}
};
System sys;


void login(int rec_sock, string username, string password) {
	char buf[1024] = "username(guest):";
	rtrim(username);
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
					close(fd);
					FD_CLR(fd, &allset);
					itr = sock_vector.erase(itr);
					continue;
				} //quit
				if (states[fd]==-1 || strncmp(buf, "quit", 4) == 0) {
					// printf("gogoin: %s\n",buf);
					// printf("Client sent 'quit'. Closing connection.\n");
					printf("close socket [%d]\n",fd);
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
						writeLine(fd, "guest mode");
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
				}
				if (states[fd]>=3){
					User *user = sys.findUserFd(fd);
					user->writef("");
					// if (user != nullptr) {
					// 	sprintf(outbuf, "<%s: %i> ", user->username.c_str(), user->cmd);
					// 	user->cmd++;
					// 	// Ensure num is the length of outbuf, not the result of the read call
					// 	write(fd, outbuf, strlen(outbuf)); 
					// } else {
					// 	printf("Error: User not found for fd: %d\n", fd);
					// }
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
}



