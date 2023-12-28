#include "websocket.h"

using namespace std;

string base64_encode(const string &original) {
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

  string encoded(bufferPtr->data, bufferPtr->length);
  BIO_free_all(bio);

  return encoded;
}

string base64_decode(const string &encoded) {
  BIO *bio, *b64;
  int decodeLen = encoded.length();
  auto *buffer = (char *)malloc(decodeLen + 1);

  bio = BIO_new_mem_buf(encoded.data(), -1);
  b64 = BIO_new(BIO_f_base64());
  BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
  bio = BIO_push(b64, bio);

  decodeLen = BIO_read(bio, buffer, encoded.length());
  buffer[decodeLen] = '\0';

  string decoded(buffer);
  free(buffer);
  BIO_free_all(bio);

  return decoded;
}

string sha1(const string &input) {
  unsigned char sha1_hash[SHA_DIGEST_LENGTH];
  SHA1((const unsigned char*)(input.c_str()), input.size(), sha1_hash);
  // SPDLOG_DEBUG("hash {}", sha1_hash);

  // cout << "SHA-1 Hash (Hexadecimal): ";
  // for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
  //   printf("%02x", sha1_hash[i]);
  // }
  // cout << endl;


  string sha1_str(reinterpret_cast<char*>(sha1_hash), SHA_DIGEST_LENGTH);
  return sha1_str;
}

map<string, string> parse_http_headers(const string& headers) {
    map<string, string> headerMap;
    istringstream stream(headers);
    string line;

    while (getline(stream, line)) {
        istringstream lineStream(line);
        string key, value;

        if (getline(lineStream, key, ':')) {
            if (getline(lineStream, value)) {
                if (!key.empty() && !value.empty()) {
                    size_t start = value.find_first_not_of(" ");
                    if (start != string::npos) {
                        value = value.substr(start);
                    }
                    headerMap[key] = value;
                }
            }
        }
    }

    return headerMap;
}

string create_websocket_response(const string& websocket_request_key) {

}
