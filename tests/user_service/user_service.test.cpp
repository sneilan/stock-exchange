#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>
#include <spdlog/spdlog.h>

#include "../../src/user_service/user_service.h"

TEST_CASE("LibSodium Test - Creating and authenticating password") {
  char hashed_password[crypto_pwhash_STRBYTES];
  memset(hashed_password, 0, crypto_pwhash_STRBYTES);

  REQUIRE(hashed_password[0] == 0);

  const char *my_password = "my_password";

  REQUIRE(crypto_pwhash_str(hashed_password, my_password, strlen(my_password),
                            crypto_pwhash_OPSLIMIT_INTERACTIVE, 
                            crypto_pwhash_MEMLIMIT_INTERACTIVE) == 0);

  REQUIRE(hashed_password[0] != 0);

  // Make sure hashed password matches hashed version.
  REQUIRE(crypto_pwhash_str_verify(hashed_password, my_password, strlen(my_password)) == 0);

  // Make sure wrong password does not.
  const char *wrong_password = "battery staple";
  REQUIRE(crypto_pwhash_str_verify(hashed_password, wrong_password, strlen(wrong_password)) != 0);
}
