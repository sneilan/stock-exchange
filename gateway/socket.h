#ifndef socket_h
#define socket_h

#include <stdio.h>
#include <string.h> //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>    //close
#include <arpa/inet.h> //close
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include "../util/spdlog/spdlog.h"

// Had some issues with spdlog levels. Will fix later.
#define DEBUG spdlog::info

#define MAX_CLIENTS 30
#define TIMEOUT_MICROSECONDS 1

class SocketServer
{
public:
    SocketServer();
    void bindSocket(int PORT);
    void listenToSocket();

private:
    int getMaxClientID(int (*client_socket)[MAX_CLIENTS]);
    int handleErrors(int i, fd_set *readfds);
    int acceptNewConn(fd_set *readfds);
    int initFDSet(fd_set *fds, int (*client_socket)[MAX_CLIENTS]);

    int master_socket, client_socket[MAX_CLIENTS];
    struct sockaddr_in address;

    char buffer[1025]; // data buffer of 1K

    // Use non-blocking sockets to wait for activity. Only wait for 1 microsecond.
    struct timeval timeout;

    char *message = "ECHO Daemon v1.0 \r\n";
};

#endif