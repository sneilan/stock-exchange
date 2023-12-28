#include "../../../src/gateway/util/websocket.h"
#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>
#include <spdlog/spdlog.h>
#include <string>

TEST_CASE("Base64 Encode/Decode") {
  std::string input = "hello world";
  std::string expected_encoding = "aGVsbG8gd29ybGQ=";
  std::string encoding_output = base64_encode(input);

  REQUIRE(encoding_output == expected_encoding);

  std::string decoding_output = base64_decode(encoding_output);
  REQUIRE(decoding_output == input);
}

TEST_CASE("HTTP Header Parsing") {
  std::string headers = 
    "Content-Type: text/html\n"
    "Content-Length: 123\n"
    "Connection: keep-alive\n"
    "Host: www.example.com";

  std::map<std::string, std::string> parsed_headers = parse_http_headers(headers);
  REQUIRE(parsed_headers.at("Content-Type") == "text/html");
}

// TEST_CASE("Sha1") {
//   std::string input = "hello world";
//   std::string expected_sha1 = "2aae6c35c94fcfb415dbe95f408b9ce91ee846ed";

//   std::string sha1_output = sha1(input);

//   REQUIRE(sha1_output == expected_sha1);
// }
