#ifndef websocket_h
#define websocket_h

#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include <spdlog/spdlog.h>
#include <string>

std::string base64_encode(const std::string &original);
std::string base64_decode(const std::string &encoded);

#endif
