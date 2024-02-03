#include "../../../src/gateway/util/websocket.h"
#include "../../../src/gateway/gateway.h"
#include "../../../src/gateway/socket.h"
#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cstring>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <signal.h>
#include <spdlog/spdlog.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>

using namespace std;

vector<uint8_t> encodeWebsocketFrame(const string &message) {
  vector<uint8_t> frame;

  // Final fragment for bit & opcode
  frame.push_back(0x81); // FIN bit (1), Opcode: Text (0x01)

  size_t payload_length = message.length();

  if (payload_length < 126) {
    frame.push_back(static_cast<uint8_t>(payload_length));
  } else if (payload_length <= 65535) {
    frame.push_back(126);
    frame.push_back(static_cast<uint8_t>((payload_length >> 8) & 0xFF));
    frame.push_back(static_cast<uint8_t>(payload_length & 0xFF));
  } else {
    /*
     * We don't do payloads above 64k in this department.
     * Why don't you try contacting customer service?
     */
  }

  for (const char &c : message) {
    frame.push_back(static_cast<uint8_t>(c));
  }

  return frame;
}

bool decodeWebSocketFrame(string frame, string &message) {
  SPDLOG_INFO("Recieved {} frame. Size {}", frame, frame.size());

  // invalid header
  if (frame.size() < 2) {
    return false;
  }

  bool fin = (frame[0] & 0x80) != 0;
  SPDLOG_INFO("Fin?: {}", fin);
  uint8_t opcode = frame[0] & 0x0F;
  SPDLOG_INFO("Opcode: {}", opcode);
  if (opcode == 0x8) {
    SPDLOG_INFO("Client disconnected using opcode 0x8");
    return false;
  }
  uint8_t payload_len = frame[1] & 0x7F;
  SPDLOG_INFO("Payload length {}", payload_len);
  size_t payload_offset = 2;

  if (payload_len == 126) {
    SPDLOG_INFO("1");
    // 16 bit payload length
    if (frame.size() < 4) {
      return false;
    }
    payload_len = (static_cast<uint16_t>(frame[2]) << 8) |
                  static_cast<uint16_t>(frame[3]);
    payload_offset = 4;
  } else if (payload_len == 127) {
    SPDLOG_INFO("2");
    // skip extended payloads
    return false;
  }

  bool maskBitSet = (frame[1] & 0x80) != 0;

  if (maskBitSet) {
    SPDLOG_INFO("3");
    if (frame.size() < payload_offset + 4) {
      return false;
    }

    string masking_key = frame.substr(payload_offset, 4);
    SPDLOG_INFO("Masking key size: {}", masking_key.size());
    SPDLOG_INFO("Masking key: {}", masking_key);
    payload_offset += 4;

    for (size_t i = payload_offset; i < payload_offset + payload_len; ++i) {
      SPDLOG_INFO("Mask Char: {}", static_cast<uint8_t>(masking_key[i % 4]));
      SPDLOG_INFO("Raw Char: {}", static_cast<uint8_t>(frame[i]));
      SPDLOG_INFO("Xored {}", static_cast<uint8_t>(frame[i]) ^
                                  static_cast<uint8_t>(masking_key[i % 4]));

      SPDLOG_INFO("Raw char {}", static_cast<uint8_t>(frame[i]) ^
                                     static_cast<uint8_t>(masking_key[i % 4]));
      frame[i] = static_cast<uint8_t>(frame[i]) ^
                 static_cast<uint8_t>(masking_key[i % 4]);
      SPDLOG_INFO("After XOR frame[{}] {}: {}", i, frame[i],
                  static_cast<uint8_t>(frame[i]));
    }
  }

  if (frame.size() < payload_offset + payload_len) {
    return false;
  }

  SPDLOG_INFO(
      "Frame size: {}, payload offset: {}, Payload length: {}, Total {} ",
      frame.size(), payload_offset, payload_len, payload_offset + payload_len);

  message.assign(frame, payload_offset, payload_len);

  SPDLOG_INFO("4");
  return true;
}

class SSLWebSocketTest : public SocketServer {
public:
  SSLWebSocketTest() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
      this->connected_webclients.push_back(false);
    }
  };
  ~SSLWebSocketTest() throw(){};

  void newClient(int client_id){
      // SPDLOG_INFO("Client {} connected", client_id);
  };

  void disconnected(int client_id){
      // SPDLOG_INFO("Client {} disconnected. Server exiting", client_id);
  };

  void readMessage(int client_id, const char *message, int message_size) {
    if (!connected_webclients[client_id]) {
      SPDLOG_INFO("Client {} has not shaken hands", client_id);

      string response = websocket_request_response(message);

      if (!sendMessage(client_id, response.c_str(), response.length())) {
        SPDLOG_INFO("Failure to send message to client {}", client_id);
        // @TODO perhaps sendMessage can handle what happens if a client
        // disconnects Then call our disconnected handler and let us know so we
        // don't have to an error handling pattern everywhere.
        forceDisconnect(client_id);
      } else {
        connected_webclients[client_id] = true;
        SPDLOG_INFO("Websocket handshake completed with {}", client_id);
      }
    } else {
      SPDLOG_INFO("Raw message from client {}", message);
      string decodedMessage;
      string frame(message, message_size);
      bool ret = decodeWebSocketFrame(message, decodedMessage);
      if (ret) {
        SPDLOG_INFO("Recieved {} from client_id {}", decodedMessage, client_id);
        vector<uint8_t> message = encodeWebsocketFrame("acknowledged.");
        const char *charArray = reinterpret_cast<const char *>(message.data());

        sendMessage(client_id, charArray, message.size());
      } else {
        SPDLOG_INFO("Could not decode packet from client_id {}", client_id);
        forceDisconnect(client_id);
      }
    }
  };

  void handleOutgoingMessage(){};

protected:
  vector<bool> connected_webclients;
};

// TEST_CASE("Base64 Encode/Decode") {
//   string input = "hello world";
//   string expected_encoding = "aGVsbG8gd29ybGQ=";
//   string encoding_output = base64_encode(input);

//   REQUIRE(encoding_output == expected_encoding);

//   string decoding_output = base64_decode(encoding_output);
//   REQUIRE(decoding_output == input);
// }

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

TEST_CASE("Websocket Client Connection") {
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
    int client_rc =
        system("/app/tests/gateway/websocket_test_client.py > /dev/stdout");

    // If status is 0 then client successfully connected & server is exiting.
    REQUIRE(client_rc == 0);

    kill(child_pid, 9);
  }
}

// TEST_CASE("Sha1") {
//   string input = "hello world";
//   string expected_sha1 = "2aae6c35c94fcfb415dbe95f408b9ce91ee846ed";

//   string sha1_output = sha1(input);

//   REQUIRE(sha1_output == expected_sha1);
// }
