#ifndef user_service_h
#define user_service_h

#include <sodium.h>

struct AuthRet {
  unsigned long long id;
  bool authenticated;
};

struct NewRet {
  unsigned long long id;
  bool created;
};

class UserService {
  UserService();
  ~UserService();
  void authenticate(char* username, char* password, AuthRet* return_val);
  void new_user(char* username, char* password, NewRet* return_val);
};

#endif
