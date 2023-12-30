#include "../../../src/gateway/util/websocket.h"
#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>
#include <spdlog/spdlog.h>
#include <string>

using namespace std;


TEST_CASE("Base64 Encode/Decode") {
  string input = "hello world";
  string expected_encoding = "aGVsbG8gd29ybGQ=";
  string encoding_output = base64_encode(input);

  REQUIRE(encoding_output == expected_encoding);

  string decoding_output = base64_decode(encoding_output);
  REQUIRE(decoding_output == input);
}

TEST_CASE("HTTP Header Parsing") {
  string headers = 
    "Content-Type: text/html\n"
    "Content-Length: 123\n"
    "Connection: keep-alive\n"
    "Host: www.example.com";

  map<string, string> parsed_headers = parse_http_headers(headers);
  REQUIRE(parsed_headers.at("Content-Type") == "text/html");
}

TEST_CASE("Websocket Request/Response") {
  string headers = 
    "GET /chat HTTP/1.1\n"
    "Host: example.com:8000\n"
    "Upgrade: websocket\n"
    "Connection: Upgrade\n"
    "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\n"
    "Sec-WebSocket-Version: 13";
  string response = websocket_request_response(headers);
  SPDLOG_DEBUG("Response {}", response);
}

// TEST_CASE("Sha1") {
//   string input = "hello world";
//   string expected_sha1 = "2aae6c35c94fcfb415dbe95f408b9ce91ee846ed";

//   string sha1_output = sha1(input);

//   REQUIRE(sha1_output == expected_sha1);
// }
