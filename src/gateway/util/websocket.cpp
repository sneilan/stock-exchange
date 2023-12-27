#include "websocket.h"

std::string base64_encode(const std::string &original) {
  BIO *bio, *b64;
  BUF_MEM *bufferPtr;

  b64 = BIO_new(BIO_f_base64());
  BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
  bio = BIO_new(BIO_s_mem());
  bio = BIO_push(b64, bio);

  BIO_write(bio, original.c_str(), original.size());
  BIO_flush(bio);
  BIO_get_mem_ptr(bio, &bufferPtr);
  BIO_set_close(bio, BIO_NOCLOSE);

  std::string encoded(bufferPtr->data, bufferPtr->length);
  BIO_free_all(bio);

  return encoded;
}

std::string base64_decode(const std::string &encoded) {
  BIO *bio, *b64;
  int decodeLen = encoded.length();
  SPDLOG_DEBUG("Encoded: {}", encoded);
  auto *buffer = (char *)malloc(decodeLen + 1);

  bio = BIO_new_mem_buf(encoded.data(), -1);
  b64 = BIO_new(BIO_f_base64());
  bio = BIO_push(b64, bio);

  decodeLen = BIO_read(bio, buffer, encoded.length());
  buffer[decodeLen] = '\0';
  SPDLOG_DEBUG("Decoded: {}", buffer);

  std::string decoded(buffer);
  free(buffer);
  BIO_free_all(bio);

  return decoded;
}
