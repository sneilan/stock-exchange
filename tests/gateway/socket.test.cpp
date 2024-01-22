#include "../../src/gateway/socket.h"
#include "../../src/gateway/gateway.h"
#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>
#include <stdlib.h>
#include <unistd.h>

class SSLSocketTest : public SocketServer {
public:
  SSLSocketTest(){};
  ~SSLSocketTest() throw(){};

  void newClient(int client_id) {
    SPDLOG_INFO("Client {} connected", client_id);
  };

  void disconnected(int client_id) {
    SPDLOG_INFO("Client {} disconnected. Server exiting", client_id);
    exit(0);
  };

  void readMessage(int client_id, char *message) {
    SPDLOG_INFO("Read {} from client {}", message, client_id);
    sendMessage(client_id, message, sizeof(message));
  };

  void handleOutgoingMessage(){};
};

TEST_CASE("Socket Server") {
  pid_t child_pid;

  // Create a new process by forking the current process
  child_pid = fork();

  if (child_pid == -1) {
    SPDLOG_ERROR("Could not fork ssl socket server");
    exit(1);
  }

  if (child_pid == 0) {
    Gateway ss;

    char *port = getenv("GATEWAY_PORT");

    ss.bindSocket(atoi(port));
    ss.listenToSocket();
  } else {
    // Attempt to connect to ssl server with test client.
    int rc = system("/app/tests/gateway/ssl_test_client.py > /dev/stdout");
    // REQUIRE(rc == 0);

    // if (kill(child_pid, SIGKILL) == -1) {
    //   perror("kill");
    //   exit(EXIT_FAILURE);
    // }

    int status;
    pid_t terminated_pid = waitpid(child_pid, &status, 0);
    // If status is 0 then client successfully connected & server is exiting.
    REQUIRE(status == 0);
  }
}
