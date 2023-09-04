#ifndef socket_h
#define socket_h

#include <arpa/inet.h> //close
#include <errno.h>
#include <netinet/in.h>
#include <spdlog/spdlog.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> //strlen
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include <sys/types.h>
#include <unistd.h> //close

#define MAX_CLIENTS 30
#define TIMEOUT_MICROSECONDS 1

class SocketServer {
public:
  SocketServer();
  void bindSocket(int PORT);
  void listenToSocket();

  // events that will be called.
  // @TODO to be implemented by gateway class.
  // Gateway should subclass the SocketServer and become a kind of SocketServer.
  virtual void newClient(int client_id) = 0;
  virtual void disconnected(int client_id) = 0;
  virtual void readMessage(int client_id, char *message) = 0;

  // Does not need to be implemented by subclass.
  bool sendMessage(int client_id, const char *message);
  void forceDisconnect(int client_id);

private:
  int getMaxClientID(int (*client_socket)[MAX_CLIENTS]);
  int handleErrors(int i, fd_set *readfds);
  void acceptNewConn(fd_set *readfds);
  void initFDSet(fd_set *fds, int (*client_socket)[MAX_CLIENTS]);

  int master_socket, client_socket[MAX_CLIENTS];
  struct sockaddr_in address;

  char buffer[1025]; // data buffer of 1K

  // Use non-blocking sockets to wait for activity. Only wait for 1 microsecond.
  struct timeval timeout;
};

#endif
