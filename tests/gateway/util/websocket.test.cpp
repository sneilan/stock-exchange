#include "../../../src/gateway/util/websocket.h"
#include "../../../src/gateway/gateway.h"
#include "../../../src/gateway/socket.h"
#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cstring>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <spdlog/spdlog.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>

using namespace std;

class SSLWebSocketTest : public SocketServer {
public:
  SSLWebSocketTest() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
      this->connected_webclients.push_back(false);
    }
  };
  ~SSLWebSocketTest() throw(){};

  void newClient(int client_id) {
    // SPDLOG_INFO("Client {} connected", client_id);
  };

  void disconnected(int client_id) {
    // SPDLOG_INFO("Client {} disconnected. Server exiting", client_id);
  };

  void readMessage(int client_id, const char *message) {
    if (!connected_webclients[client_id]) {
      SPDLOG_INFO("Client {} has not shaken hands", client_id);

      string response = websocket_request_response(message);

      if (!sendMessage(client_id, response.c_str(), response.length())) {
        // SPDLOG_INFO("Failure to send message to client {}", client_id);
        // @TODO perhaps sendMessage can handle what happens if a client
        // disconnects Then call our disconnected handler and let us know so we
        // don't have to an error handling pattern everywhere.
        // forceDisconnect(client_id);
      } else {
        connected_webclients[client_id] = true;
        SPDLOG_INFO("Websocket handshake completed with {}", client_id);
        // const char * meow = "meowjlkasjdflkasjdflkajsdflkjasdlfky7o8234";
        // sendMessage(client_id, meow, (int)strlen(meow));
      }
    } else {
      SPDLOG_INFO("Recieved {} from client_id {}", message, client_id);
      sendMessage(client_id, message, (int)strlen(message));
    }
  };

  void handleOutgoingMessage(){};

protected:
  vector<bool> connected_webclients;
};

TEST_CASE("Base64 Encode/Decode") {
  string input = "hello world";
  string expected_encoding = "aGVsbG8gd29ybGQ=";
  string encoding_output = base64_encode(input);

  REQUIRE(encoding_output == expected_encoding);

  string decoding_output = base64_decode(encoding_output);
  REQUIRE(decoding_output == input);
}

TEST_CASE("HTTP Header Parsing") {
  string headers = "Content-Type: text/html\n"
                   "Content-Length: 123\n"
                   "Connection: keep-alive\n"
                   "Host: www.example.com";

  map<string, string> parsed_headers = parse_http_headers(headers);
  REQUIRE(parsed_headers.at("Content-Type") == "text/html");
}

TEST_CASE("Websocket Request/Response") {
  string headers = "GET /chat HTTP/1.1\n"
                   "Host: example.com:8000\n"
                   "Upgrade: websocket\n"
                   "Connection: Upgrade\n"
                   "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\n"
                   "Sec-WebSocket-Version: 13";
  string response = websocket_request_response(headers);
  SPDLOG_DEBUG("Response {}", response);
}

TEST_CASE("ASDFWebsocket Client Connection") {
  pid_t child_pid;

  // Create a new process by forking the current process
  child_pid = fork();

  if (child_pid == -1) {
    SPDLOG_ERROR("Could not fork ssl socket server");
    exit(1);
  }

  if (child_pid == 0) {
    SSLWebSocketTest ss;

    char *port = getenv("GATEWAY_PORT");

    ss.bindSocket(atoi(port));
    ss.listenToSocket();
  } else {
    // Attempt to connect to ssl server with test client.
    int client_rc = system("/app/tests/gateway/websocket_test_client.py > /dev/stdout");

    // If status is 0 then client successfully connected & server is exiting.
    REQUIRE(client_rc == 0);
  }
}

TEST_CASE("Sha1") {
  string input = "hello world";
  string expected_sha1 = "2aae6c35c94fcfb415dbe95f408b9ce91ee846ed";

  string sha1_output = sha1(input);

  REQUIRE(sha1_output == expected_sha1);
}
