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

using namespace std;

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

    User(string username, string password) {
        this->username = username;
        this->password = password;
        information = username;
        quiet = false;
        login = false;
        sockId = -1;
    }
	User() {
        this->username = username;
        this->password = password;
        information = username;
        quiet = false;
        login = false;
        sockId = -1;
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
			cout << "*********************************************" <<endl;
			cout << "username:" << user->username<<endl;
			cout << "password:" << user->password<<endl;
			cout << "id:" << user->id<<endl;
			cout << "info:" << user->information<<endl;
			cout << "rating:" << user->rating<<endl;
			cout << "win:" << user->win<<endl;
			cout << "loss:" << user->loss<<endl;
			cout << "quiet:" << user->quiet<<endl;
			cout << "block:";
			for (const auto& num : user->blocked_ids) {
				cout << num << ",";
			}
			cout << endl;
		}
		file.close();
	}
};
System sys;



void login(int rec_sock) {
	char user[100] = "dandan";
	char password[100] = "dandan";
    char hello[1024];
	char buf[100];
	int num;
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
	strcpy(buf,"username:");
	write(rec_sock, buf, strlen(buf));
	printf("buf1:[%s]\n",buf);
	num = read(rec_sock, buf, 100);
	if (num > 0) {
		buf[num] = '\0'; // Null-terminate the string
	}
	printf("buf2:[%s]\n",buf);
	write(rec_sock, buf, strlen(buf));
}

char* print_hello() {
    // Make the buffer static so its lifetime extends beyond the function call.
    // Note: This makes the function non-reentrant and not thread-safe.
    static char buf[1024];
    sprintf(buf, 
        "%s%s%s%s%s",
        "********************************************************************\n",
        "You are attempting to log into online tic-tac-toe Server.\n",
        "Please be advised by continuing that you agree to the terms of the\n",
        "Computer Access and Usage Policy of online tic-tac-toe Server.\n\n",
        "********************************************************************\n\n\n");
    return buf;
}

void start_server(char* port) {
	int sockfd, rec_sock;
	socklen_t len;
	vector<int> sock_vector;
	struct sockaddr_in addr, recaddr;
	struct sigaction abc;
	char buf[100];
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
    printf("init sockfd[%d]\n",sockfd);

	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_family = AF_INET;
	addr.sin_port = htons((short)atoi(port));

	if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror(": bind");
		exit(1);
	}
	printf("bind sockfd[%d]\n",sockfd);


	len = sizeof(addr);
	if (getsockname(sockfd, (struct sockaddr *)&addr, &len) < 0) {
		perror(": can't get name");
		_exit(1);
	}
	printf("getsockname sockfd[%d]\n",sockfd);

	printf("ip = %s, port = %d\n", inet_ntoa(addr.sin_addr), htons(addr.sin_port));

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
			printf("I am ISSET before accept\n");
			printf("before ip = %s, port = %d\n", inet_ntoa(recaddr.sin_addr), htons(recaddr.sin_port));
			if ((rec_sock = accept(sockfd, (struct sockaddr *)(&recaddr), &len)) < 0) {
				if (errno == EINTR)
					continue;
				else {
					perror(":accept error");
					exit(1);
				}
			}
			printf("after ip = %s, port = %d\n", inet_ntoa(recaddr.sin_addr), htons(recaddr.sin_port));
            printf("rec_sock[%d]\n", rec_sock);
			login(rec_sock);
            // char* buf = print_hello();
            // printf("%s\n",buf);
            // if (write(rec_sock, buf, strlen(buf)) < 0) {
            //     perror("hello error");
            // }
            

			/*
			if (rec_sock < 0) {
				perror(": accept");
				exit(1);
			}
			*/

			/* print the remote socket information */

			printf("remote machine = %s, port = %d.\n",
					inet_ntoa(recaddr.sin_addr), ntohs(recaddr.sin_port)); 

			sock_vector.push_back(rec_sock);
			FD_SET(rec_sock, &allset);
			if (rec_sock > maxfd) maxfd = rec_sock;
		}

		// vector<int>::iterator itr = sock_vector.begin();
		auto itr = sock_vector.begin(); 
		while (itr != sock_vector.end()) {
			int num, fd;
			fd = *itr;
			if (FD_ISSET(fd, &rset)) {
				num = read(fd, buf, 100);
				if (num == 0) {
					/* client exits */
					close(fd);
					FD_CLR(fd, &allset);
					itr = sock_vector.erase(itr);
					continue;
				} else {
					write(fd, buf, num);
					if (strncmp(buf, "quit", 4) == 0) {
						printf("gogoin: %s\n",buf);
						printf("Client sent 'quit'. Closing connection.\n");
						close(rec_sock);
						FD_CLR(fd, &allset);
						itr = sock_vector.erase(itr);
						continue;
					}
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
	// start_server(argv[1]);
}



