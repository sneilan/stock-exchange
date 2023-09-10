#include "socket.h"

void SocketServer::listenToSocket() {
  // accept the incoming connection
  // int addrlen = sizeof(address);

  SPDLOG_INFO("Waiting for connections ...");
  while (1) {
    // set of socket descriptors for sockets with data to be read.
    fd_set readfds;
    // set of socket descriptors for sockets that can be written to.
    fd_set writefds;

    int max_sd = getMaxClientID(&client_socket);
    initFDSet(&readfds, &client_socket);
    initFDSet(&writefds, &client_socket);

    int activity = select(max_sd + 1, &readfds, &writefds, NULL, &timeout);

    if ((activity < 0) && (errno != EINTR)) {
      SPDLOG_ERROR("select error");
    }

    // non-blocking accept new connection call.
    acceptNewConn(&readfds);

    // else its some IO operation on some other socket
    for (int i = 0; i < MAX_CLIENTS; i++) {
      int sd = client_socket[i];

      if (FD_ISSET(sd, &readfds)) {
        // @TODO handleErrors is also responsible for reading data from the
        // client.
        int valread = handleErrors(i, &readfds);

        if (valread > 0) {
          // set the string terminating NULL byte on the end
          // of the data read
          buffer[valread] = '\0';
          SPDLOG_INFO("Calling readMessage from client_id {}", i);
          readMessage(i, buffer);
          memset(buffer, 0, sizeof(buffer));
        } else {
          // handleErrors unsets readfds but we also need to unset writefds
          // @TODO this is weird because I expect handleErrors to handle all
          // sorts of things by how generic the name is. I shouldn't have to do
          // extra work to handle errors outside of handleErrors.
          FD_CLR(sd, &writefds);
        }
      }
    }
  }
}

SocketServer::SocketServer() {
  timeout.tv_sec = 0;
  timeout.tv_usec = TIMEOUT_MICROSECONDS;

  // initialise all client_socket[] to 0 so not checked
  for (int i = 0; i < MAX_CLIENTS; i++) {
    client_socket[i] = 0;
  }
}

void SocketServer::forceDisconnect(int client_id) {
  close(client_socket[client_id]);
  client_socket[client_id] = 0;
}

void SocketServer::bindSocket(int PORT) {
  // create a master socket
  if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    SPDLOG_DEBUG("socket failed");
    exit(EXIT_FAILURE);
  }

  // set master socket to allow multiple connections ,
  // this is just a good habit, it will work without this
  int opt = 1;
  if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
                 sizeof(opt)) < 0) {
    SPDLOG_DEBUG("setsockopt");
    exit(EXIT_FAILURE);
  }

  // type of socket created
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  // bind the socket to localhost port 8888
  if (bind(master_socket, (struct sockaddr *)&address, sizeof(address)) < 0) {
    SPDLOG_DEBUG("bind failed");
    exit(EXIT_FAILURE);
  }

  SPDLOG_INFO("Listener on port {}", PORT);
  // try to specify maximum of 3 pending connections for the master socket
  if (listen(master_socket, 3) < 0) {
    SPDLOG_DEBUG("listen failure");
    exit(EXIT_FAILURE);
  }
}

int SocketServer::getMaxClientID(int (*client_socket)[MAX_CLIENTS]) {
  // Return the largest client socket id of all client sockets.
  int max_sd = master_socket;

  for (int i = 0; i < MAX_CLIENTS; i++) {
    int sd = (*client_socket)[i];
    if (sd > max_sd)
      max_sd = sd;
  }

  return max_sd;
}

void SocketServer::initFDSet(fd_set *fds, int (*client_socket)[MAX_CLIENTS]) {
  // clear the socket set
  FD_ZERO(fds);

  // add master socket to set
  FD_SET(master_socket, fds);

  // add child sockets to set
  for (int i = 0; i < MAX_CLIENTS; i++) {
    // socket descriptor
    int sd = (*client_socket)[i];

    // if valid socket descriptor then add to read list
    if (sd > 0)
      FD_SET(sd, fds);
  }
}

void SocketServer::acceptNewConn(fd_set *readfds) {
  int new_socket = 0;
  int addrlen = sizeof(address);

  if (FD_ISSET(master_socket, readfds)) {
    if ((new_socket = accept(master_socket, (struct sockaddr *)&address,
                             (socklen_t *)&addrlen)) < 0) {
      SPDLOG_DEBUG("accept");
      exit(EXIT_FAILURE);
    }

    // add new socket to array of sockets
    for (int i = 0; i < MAX_CLIENTS; i++) {
      // if position is empty
      if (client_socket[i] == 0) {
        client_socket[i] = new_socket;
        newClient(i);
        SPDLOG_DEBUG("Registering new client {}", i);

        break;
      }
    }
  }
}

bool SocketServer::sendMessage(int client_id, const char *message) {
  int error = send(client_socket[client_id], message, strlen(message), 0);
  if (error != strlen(message)) {
    SPDLOG_ERROR("send error {} to client_id {} at socket {}", error, client_id, client_socket[client_id]);
    return false;
  }

  SPDLOG_INFO("sent message {} to client {}", message, client_id);

  return true;
}

// @TODO make buffer a pointer to this.
// @TODO why is handleErrors also responsible for reading data from the client?
int SocketServer::handleErrors(int i, fd_set *readfds) {
  int valread;
  int addrlen = sizeof(address);
  int sd = client_socket[i];

  // Check if it was for closing , and also read the
  // incoming message
  if ((valread = read(sd, buffer, 1024)) == 0) {
    // Somebody disconnected , get the details and print
    getpeername(sd, (struct sockaddr *)&address, (socklen_t *)&addrlen);

    // Close the socket and mark as 0 in list for reuse
    close(sd);
    client_socket[i] = 0;
    disconnected(i);
    SPDLOG_DEBUG("Host disconnected, ip {}, port {}, client {}",
                  inet_ntoa(address.sin_addr), ntohs(address.sin_port), i);
  }
  SPDLOG_DEBUG("Read valread {} bytes from client_id {}", valread, i);

  /*
    else if (valread == -1)
    {
        // @TODO do not return 3x. This is terrible.
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            // The socket does not have any data to be read. Try again later.
            // @TODO may not be necesary if we are using non-blocking sockets
            // because we will only get data for sockets that have data.
            return valread;
        }
        else
        {
            // @TODO handle other errors here
            return valread;
        }
    }
    */

  return valread;
}
