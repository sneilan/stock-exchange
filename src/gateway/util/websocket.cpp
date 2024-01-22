#include "websocket.h"

using namespace std;

string base64_encode_c(unsigned char * original) {
  BIO *bio, *b64;
  BUF_MEM *bufferPtr;

  b64 = BIO_new(BIO_f_base64());
  BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
  bio = BIO_new(BIO_s_mem());
  bio = BIO_push(b64, bio);

  BIO_write(bio, original, SHA_DIGEST_LENGTH);
  BIO_flush(bio);
  BIO_get_mem_ptr(bio, &bufferPtr);
  BIO_set_close(bio, BIO_NOCLOSE);

  string encoded(bufferPtr->data, bufferPtr->length);
  BIO_free_all(bio);

  return encoded;
}

// Did not encode correctly according to websocket test.
// string base64_encode(const string &original) {
//   BIO *bio, *b64;
//   BUF_MEM *bufferPtr;

//   b64 = BIO_new(BIO_f_base64());
//   BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
//   bio = BIO_new(BIO_s_mem());
//   bio = BIO_push(b64, bio);

//   BIO_write(bio, original.c_str(), original.size());
//   BIO_flush(bio);
//   BIO_get_mem_ptr(bio, &bufferPtr);
//   BIO_set_close(bio, BIO_NOCLOSE);

//   string encoded(bufferPtr->data, bufferPtr->length);
//   BIO_free_all(bio);

//   return encoded;
// }

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
  SHA1((const unsigned char *)(input.c_str()), input.size(), sha1_hash);

  string ret(reinterpret_cast<char*>(sha1_hash));
  return ret;

  // stringstream ss;
  // ss << hex << setfill('0');
  // for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
  //   ss << setw(2) << static_cast<unsigned int>(sha1_hash[i]);
  // }

  // return ss.str();
}

map<string, string> parse_http_headers(const string &headers) {
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

string create_websocket_response_nonce(const string &websocket_request_key) {
  string result = websocket_request_key + ws_magic_string;

  unsigned char sha1_hash[SHA_DIGEST_LENGTH];
  SHA1((const unsigned char *)(result.c_str()), result.size(), sha1_hash);

  return base64_encode_c(sha1_hash);
}

string websocket_request_response(const string &client_http_request) {
  // SPDLOG_DEBUG("Client Http Request Raw {}", client_http_request);

  map<string, string> http_headers = parse_http_headers(client_http_request);

  // for (const auto& pair : http_headers) {
  //   SPDLOG_DEBUG("{} : {}", pair.first, pair.second);
  // }

  try {
    string ws_request_nonce = http_headers.at(ws_request_header);

    ws_request_nonce.erase(std::remove_if(ws_request_nonce.begin(), ws_request_nonce.end(),
                                          [](char c) { return c == '\n' || c == '\r'; }),
                           ws_request_nonce.end());
    string ws_response_nonce = create_websocket_response_nonce(ws_request_nonce);
    return ws_response + ws_response_nonce + "\nSec-WebSocket-Protocol: chat\n\n";
  } catch (const out_of_range& e) {
    // Make client disconnect here.
    return "meow";
  }
}
