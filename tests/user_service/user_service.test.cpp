#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>
#include <csignal>
#include <spdlog/spdlog.h>
#include <cstdio>
#include <stdlib.h>

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

TEST_CASE("Sqlite3 - Smoke test") {
  sqlite3* db;
  const char * db_name = "/tmp/test.db";

  // cleanup just in case
  std::remove(db_name);

  int rc = sqlite3_open(db_name, &db);

  REQUIRE(rc == 0);

  sqlite3_close(db);
  std::remove(db_name);
}

TEST_CASE("UserService - Create database, add user and try logging in") {
  const char * db_name = "/tmp/users.db";

  // cleanup just in case
  std::remove(db_name);

  int rc = system("/app/scripts/operations/create_databases.py --user-db /tmp/users.db");
  REQUIRE(rc == 0);

  rc = system("/app/scripts/operations/add_user.py --username sneilan --password password --user-db /tmp/users.db");
  REQUIRE(rc == 0);

  // test a good password
  UserService* user_service = new UserService(db_name);
  AuthRet ret;
  ret.authenticated = false;
  char username[] = "sneilan";
  char password[] = "password";
  user_service->authenticate(username, password, &ret);

  REQUIRE(ret.authenticated == true);

  // Test a bad password
  char bad_password[] = "bad_password";
  ret.authenticated = false;
  user_service->authenticate(username, bad_password, &ret);

  REQUIRE(ret.authenticated == false);

  // Test what happens when user does not exist
  ret.authenticated = false;
  char bad_username[] = "bad_username";
  user_service->authenticate(bad_username, password, &ret);

  REQUIRE(ret.authenticated == false);

  std::remove(db_name);
}
