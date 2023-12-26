#include "socket.h"

SocketServer::SocketServer() {
  timeout.tv_sec = 0;
  timeout.tv_usec = TIMEOUT_MICROSECONDS;

  // Initialise all client_socket[] to 0
  for (int i = 0; i < MAX_CLIENTS; i++) {
    client_socket[i] = 0;
  }

  // Default to SSL.
 
  ctx = SSL_CTX_new(SSLv23_server_method());

  if (!ctx) {
    SPDLOG_CRITICAL("Could not start SSL_CTX_new");
    exit(1);
  }

  if (SSL_CTX_use_certificate_file(ctx, getenv("CERTIFICATE"), SSL_FILETYPE_PEM) <= 0) {
    SPDLOG_CRITICAL("Could not load certificate. Check CERTIFICATE env variable.");
    exit(1);
  }

  if (SSL_CTX_use_PrivateKey_file(ctx, getenv("PRIVATE_KEY"), SSL_FILETYPE_PEM) <= 0) {
    SPDLOG_CRITICAL("Could not load private key. Check PRIVATE_KEY env variable.");
    exit(1);
  }

  // Initialize ssl connection pool
  for (int i = 0; i < MAX_CLIENTS; i++) {
    connections[i] = SSL_new(ctx);
  }
}

void SocketServer::listenToSocket() {
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

    // if activity is greater than 0, 1 or more socket descriptors
    // ready for reading, writing or error. 0 means no socket descriptors
    // ready and we can try again later. -1 means an error happened.
    // When -1 happens, call errno to determine the error type.
    // EINTR means interrupted system call. All errno's are defined here.
    // https://man7.org/linux/man-pages/man3/errno.3.html
    if ((activity < 0) && (errno != EINTR)) {
      SPDLOG_ERROR("select error {}", errno);
    }

    // Accept new connections non-blocking.
    acceptNewConn(&readfds);

    // Check all sockets for data to be read.
    for (int i = 0; i < MAX_CLIENTS; i++) {
      int sd = client_socket[i];

      if (FD_ISSET(sd, &readfds)) {
        int valread = readDataFromClient(i);

        if (valread > 0) {
          // Set the string terminating NULL byte on the end of the data read
          buffer[valread] = '\0';
          SPDLOG_INFO("Calling readMessage from client_id {}", i);
          readMessage(i, buffer);
          memset(buffer, 0, sizeof(buffer));
        } else {
          // If no data was read, unset writefds.
          FD_CLR(sd, &writefds);
        }
      }
    }

    handleOutgoingMessage();
  }
}

SocketServer::~SocketServer() { 
  for (int i = 0; i < MAX_CLIENTS; i++) {
    SSL_free(connections[i]);
  }

  SSL_CTX_free(ctx);
}

void SocketServer::forceDisconnect(int client_id) {
  close(client_socket[client_id]);
  client_socket[client_id] = 0;
}

void SocketServer::bindSocket(int PORT) {
  // Create a master socket
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

  // Set up the address where we will host the exchange.
  address.sin_family = AF_INET; // ipv4
  address.sin_addr.s_addr = INADDR_ANY; // bind to all interfaces
  address.sin_port = htons(PORT); // bind to PORT

  // Bind master socket to that address.
  if (bind(master_socket, (struct sockaddr *)&address, sizeof(address)) < 0) {
    SPDLOG_DEBUG("bind failed");
    exit(EXIT_FAILURE);
  }

  SPDLOG_INFO("Listener on port {}", PORT);
  // Specify maximum of 3 pending connections for the master socket
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
      SPDLOG_ERROR("Failure to accept new connection.");
      exit(EXIT_FAILURE);
    }

    // Add new socket to array of sockets
    // Find the lowest available socket.
    for (int i = 0; i < MAX_CLIENTS; i++) {
      if (client_socket[i] == 0) {
        SSL * ssl = connections[i];

        SSL_clear(ssl);

        SSL_set_fd(ssl, new_socket);

        // Perform SSL handshake
        if (SSL_accept(ssl) != 1) {
          SPDLOG_INFO("Client {} did not accept ssl", i);
          forceDisconnect(i);
          return;
        }

        client_socket[i] = new_socket;

        newClient(i);
        SPDLOG_DEBUG("Registered new client {}", i);

        break;
      }
    }
  }
}

bool SocketServer::sendMessage(int client_id, char *message, int message_size) {
  int bytes_written = SSL_write(connections[client_id], message, message_size);
  // send(client_socket[client_id], message, message_size, 0);
 
  if (bytes_written == 0) {
    // client disconnected.
    return false;
  } else if (bytes_written != message_size) {
    // if error is not EAWOULDBLOCK, client is disconnected.
    // if EAWOULDBLOCK then have to repeat.
    SPDLOG_ERROR("send error {} to client_id {} at socket {}", bytes_written, client_id,
                 client_socket[client_id]);
    return false;
  }

  SPDLOG_INFO("Sent message {} to client {}", message, client_id);

  return true;
}

void SocketServer::sendMessageToAllClients(char* message, int message_size) {
  for (int i = 0; i < MAX_CLIENTS; i++) {
    int sd = client_socket[i];
    if (sd == 0) {
      continue;
    }

    // @TODO use ssl write here
    int error = send(sd, message, message_size, 0);
    if (error != message_size) {
      SPDLOG_ERROR("Send error {} to client_id {} at socket {}", error, i, sd);
    }
  }
}

int SocketServer::readDataFromClient(int client_id) {
  int valread;
  int sd = client_socket[client_id];
  int addrlen = sizeof(address);

  if ((valread = SSL_read(connections[client_id], buffer, 1024)) == 0) {
    // Client disconnected.
    getpeername(sd, (struct sockaddr *)&address, (socklen_t *)&addrlen);

    close(sd);
    client_socket[client_id] = 0;
    disconnected(client_id);
    SPDLOG_DEBUG("Client disconnected, ip {}, port {}, client {}",
                 inet_ntoa(address.sin_addr), ntohs(address.sin_port), client_id);
  }

  SPDLOG_DEBUG("Read valread {} bytes from client_id {}", valread, client_id);

  return valread;
}
