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
  auto *buffer = (char *)malloc(decodeLen + 1);

  bio = BIO_new_mem_buf(encoded.data(), -1);
  b64 = BIO_new(BIO_f_base64());
  BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
  bio = BIO_push(b64, bio);

  decodeLen = BIO_read(bio, buffer, encoded.length());
  buffer[decodeLen] = '\0';

  std::string decoded(buffer);
  free(buffer);
  BIO_free_all(bio);

  return decoded;
}

std::string sha1(const std::string &input) {
  unsigned char sha1_hash[SHA_DIGEST_LENGTH];
  SHA1((const unsigned char*)(input.c_str()), input.size(), sha1_hash);
  // SPDLOG_DEBUG("hash {}", sha1_hash);

  // std::cout << "SHA-1 Hash (Hexadecimal): ";
  // for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
  //   printf("%02x", sha1_hash[i]);
  // }
  // std::cout << std::endl;


  std::string sha1_str(reinterpret_cast<char*>(sha1_hash), SHA_DIGEST_LENGTH);
  return sha1_str;
}

std::map<std::string, std::string> parse_http_headers(const std::string& headers) {
    std::map<std::string, std::string> headerMap;
    std::istringstream stream(headers);
    std::string line;

    while (std::getline(stream, line)) {
        std::istringstream lineStream(line);
        std::string key, value;

        if (std::getline(lineStream, key, ':')) {
            if (std::getline(lineStream, value)) {
                if (!key.empty() && !value.empty()) {
                    size_t start = value.find_first_not_of(" ");
                    if (start != std::string::npos) {
                        value = value.substr(start);
                    }
                    headerMap[key] = value;
                }
            }
        }
    }

    return headerMap;
}
