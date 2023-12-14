#include "user_service.h"

UserService::UserService(const char* user_db_location) {
  this->user_db_location = user_db_location;
  this->open_database();
}

UserService::UserService() {
  this->user_db_location = getenv("USER_DATABASE");
  this->open_database();
}

void UserService::open_database() {
  int rc = sqlite3_open_v2(user_db_location, &db, SQLITE_OPEN_READONLY, nullptr);

  if (rc) {
    SPDLOG_CRITICAL("Can't open database {} ❌", sqlite3_errmsg(db));
    throw std::runtime_error("Can't open user database");
  }
}

UserService::~UserService() {
  sqlite3_close(db);
}

void UserService::authenticate(char* username, char* password, AuthRet* return_val) {
  const char *sql = "SELECT id, password FROM users WHERE username = ?";

  sqlite3_stmt *stmt;

  int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

  if (rc != SQLITE_OK) {
    SPDLOG_CRITICAL("Can't prepare statement {}", sqlite3_errmsg(db));
    sqlite3_close(db);
    throw std::runtime_error("Can't prepare user selection statement.");
  }

  rc = sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);

  if (rc != SQLITE_OK) {
    SPDLOG_CRITICAL("Can't bind username to stmt {}", sqlite3_errmsg(db));
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    throw std::runtime_error("Can't username to stmt.");
  }

  return_val->authenticated = false;

  while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
    // Process the results here
    return_val->id = sqlite3_column_int(stmt, 0);
    const char *hashed_password = (const char *)sqlite3_column_text(stmt, 1);

    if (crypto_pwhash_str_verify(hashed_password, password, strlen(password)) == 0) {
      return_val->authenticated = true;
      return;
    }
  }

  if (rc != SQLITE_DONE) {
    SPDLOG_CRITICAL("SQL error {}", sqlite3_errmsg(db));
  }
}
