#ifndef user_service_h
#define user_service_h

#include <sodium.h>
#include <sqlite3.h>

struct AuthRet {
  unsigned long long id;
  bool authenticated;
};

class UserService {
  UserService();
  ~UserService();
  void authenticate(char* username, char* password, AuthRet* return_val);
};

#endif
