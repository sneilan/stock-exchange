#ifndef websocket_h
#define websocket_h

#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <spdlog/spdlog.h>
#include <string>
#include <iostream>
#include <sstream>
#include <map>

using namespace std;

// RFC-6455 specifies that we add this string to the websocket request nonce
static const char * MAGIC_STRING = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

string base64_encode(const string &original);
string base64_decode(const string &encoded);
string sha1(const string &input);
map<string, string> parse_http_headers(const string& headers);
string create_websocket_response(const string& websocket_request_key);

#endif
