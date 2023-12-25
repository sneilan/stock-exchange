#include "../../src/gateway/socket.h"
#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>

class SSLSocketTest : public SocketServer {
public:
  SSLSocketTest();
  ~SSLSocketTest() throw();

  void newClient(int client_id) override;
  void disconnected(int client_id) override;
  void readMessage(int client_id, char *message) override;
  // Whenever a message goes into the outgoing ring buffer
  // this function is called to send a message to the client.
  void handleOutgoingMessage() override;
  void run();
};

TEST_CASE("SSL Socket Server Startup") { SSLSocketTest ss; }
