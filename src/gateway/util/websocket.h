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

std::string base64_encode(const std::string &original);
std::string base64_decode(const std::string &encoded);
std::string sha1(const std::string &input);
std::map<std::string, std::string> parse_http_headers(const std::string& headers);

#endif
