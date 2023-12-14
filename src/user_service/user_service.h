#ifndef user_service_h
#define user_service_h

#include <sodium.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <cstring>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <exception>

struct AuthRet {
  unsigned long long id;
  bool authenticated;
};

class UserService {
public:
  UserService(const char * user_db_location);
  UserService();
  ~UserService();
  void authenticate(char* username, char* password, AuthRet* return_val);
  void set_db_location(char * db_location);
  const char * user_db_location;
  void open_database();
  sqlite3 *db;
};

#endif
