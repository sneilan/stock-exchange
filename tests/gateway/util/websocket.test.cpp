#include "../../../src/gateway/util/websocket.h"
#include "../../../src/gateway/gateway.h"
#include "../../../src/gateway/socket.h"
#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>
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
    fill(connected_webclients.begin(), connected_webclients.end(), false);
  };
  ~SSLWebSocketTest() throw(){};

  void newClient(int client_id) {
    SPDLOG_INFO("Client {} connected", client_id);
  };

  void disconnected(int client_id) {
    SPDLOG_INFO("Client {} disconnected. Server exiting", client_id);
    exit(0);
  };

  void readMessage(int client_id, char *message) {
    string response = websocket_request_response(message);

    if (!sendMessage(client_id, response.c_str(), response.length())) {
      // @TODO perhaps sendMessage can handle what happens if a client
      // disconnects Then call our disconnected handler and let us know so we
      // don't have to an error handling pattern everywhere.
      forceDisconnect(client_id);
    } else {
      connected_webclients[client_id] = true;
      SPDLOG_INFO("Websocket handshake completed with {}", client_id);
    }

    return;
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
    int rc = system("/app/tests/gateway/ssl_test_client.py");

    int status;
    pid_t terminated_pid = waitpid(child_pid, &status, 0);
    // If status is 0 then client successfully connected & server is exiting.
    REQUIRE(status == 0);
  }
}

// TEST_CASE("Sha1") {
//   string input = "hello world";
//   string expected_sha1 = "2aae6c35c94fcfb415dbe95f408b9ce91ee846ed";

//   string sha1_output = sha1(input);

//   REQUIRE(sha1_output == expected_sha1);
// }
