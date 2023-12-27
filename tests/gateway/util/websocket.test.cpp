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
