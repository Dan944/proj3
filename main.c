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

using namespace std;

void sig_chld(int signo) {
    pid_t pid;
    int stat;
    while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
        printf("child %d terminated.\n", pid);
    return;
}

int main(int argc, char *argv[]) {
    int sockfd, rec_sock;
    socklen_t len;
    vector<int> sock_vector;
    struct sockaddr_in addr, recaddr;
    struct sigaction abc;
    char buf[100] = "Hello World\n"; // Buffer now contains "Hello World\n"
    char buf2[100];
    fd_set allset, rset;
    int maxfd;

    abc.sa_handler = sig_chld;
    sigemptyset(&abc.sa_mask);
    abc.sa_flags = 0;
    sigaction(SIGCHLD, &abc, NULL);

    if (argc < 2) {
        printf("Usage: %s port.\n", argv[0]); // Use argv[0] for the program name
        exit(0);
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror(": Can't get socket");
        exit(1);
    }

    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_family = AF_INET;
    addr.sin_port = htons((short)atoi(argv[1]));

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror(": bind");
        exit(1);
    }

    if (listen(sockfd, 5) < 0) {
        perror(": listen");
        exit(1);
    }

    FD_ZERO(&allset);
    FD_SET(sockfd, &allset);
    maxfd = sockfd;

    while (1) {
        rset = allset;
        if (select(maxfd + 1, &rset, NULL, NULL, NULL) < 0) {
            perror(": select error");
            exit(1);
        }

        if (FD_ISSET(sockfd, &rset)) {
            len = sizeof(recaddr);
            if ((rec_sock = accept(sockfd, (struct sockaddr *)(&recaddr), &len)) < 0) {
                if (errno == EINTR)
                    continue;
                else {
                    perror(": accept error");
                    exit(1);
                }
            }

            printf("Connection from %s, port %d.\n", inet_ntoa(recaddr.sin_addr), ntohs(recaddr.sin_port));

            // Receive message from client
            char client_msg[1024];
            memset(client_msg, 0, sizeof(client_msg)); // Clear the buffer
            ssize_t received_bytes = recv(rec_sock, client_msg, sizeof(client_msg) - 1, 0); // Leave space for null terminator
            if (received_bytes < 0) {
                perror("recv error");
            } else if (received_bytes == 0) {
                printf("Client closed the connection.\n");
            } else {
                // Check if the received message is "quit"
                if (strncmp(client_msg, "quit", 4) == 0) {
                    printf("gogoin: %s\n",client_msg);
                    printf("Client sent 'quit'. Closing connection.\n");
                    close(rec_sock); // Close the connection
                    continue; // Skip the rest of the loop and wait for another connection
                }

                // Echo the received message back to the client
                if (send(rec_sock, client_msg, received_bytes, 0) < 0) {
                    perror("send error");
                }
            }

            // close(rec_sock); 
        }
    }
}
