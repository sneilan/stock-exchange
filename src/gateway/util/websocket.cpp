#include "websocket.h"

using namespace std;

string base64_encode(unsigned char *original) {
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

  string ret(reinterpret_cast<char *>(sha1_hash));
  return ret;
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

  return base64_encode(sha1_hash);
}

string websocket_request_response(const string &client_http_request) {
  map<string, string> http_headers = parse_http_headers(client_http_request);

  try {
    string ws_request_nonce = http_headers.at(ws_request_header);

    ws_request_nonce.erase(
        std::remove_if(ws_request_nonce.begin(), ws_request_nonce.end(),
                       [](char c) { return c == '\n' || c == '\r'; }),
        ws_request_nonce.end());
    string ws_response_nonce =
        create_websocket_response_nonce(ws_request_nonce);
    return ws_response + ws_response_nonce +
           "\nSec-WebSocket-Protocol: chat\n\n";
  } catch (const out_of_range &e) {
    // @TODO Make client disconnect here.
    return "meow";
  }
}

void printStringAsHex(const string &str) {
  for (size_t i = 0; i < str.size(); ++i) {
    cout << hex << setw(2) << setfill('0') << static_cast<int>(str[i]) << " ";
  }

  cout << endl;
}

void printByteAsHex(const string &message, const uint8_t byte) {
  cout << message << hex << setw(2) << setfill('0') << static_cast<int>(byte)
       << " ";
  cout << endl;
}

vector<uint8_t> encodeWebsocketFrame(const string &message) {
  vector<uint8_t> frame;

  // Final fragment for bit & opcode
  frame.push_back(0x81); // FIN bit (1), Opcode: Text (0x01)

  size_t payload_length = message.length();

  if (payload_length < 126) {
    frame.push_back(static_cast<uint8_t>(payload_length));
  } else if (payload_length <= 65535) {
    frame.push_back(126);
    frame.push_back(static_cast<uint8_t>((payload_length >> 8) & 0xFF));
    frame.push_back(static_cast<uint8_t>(payload_length & 0xFF));
  } else {
    /*
     * We don't do payloads above 64k in this department.
     * Why don't you try contacting customer service?
     */
  }

  for (const char &c : message) {
    frame.push_back(static_cast<uint8_t>(c));
  }

  return frame;
}

bool decodeWebSocketFrame(string frame, string &message) {
  // invalid header
  if (frame.size() < 2) {
    return false;
  }

  bool fin = (frame[0] & 0x80) != 0;
  uint8_t opcode = frame[0] & 0x0F;
  if (opcode == 0x8) {
    SPDLOG_INFO("Client disconnected using opcode 0x8");
    return false;
  }
  uint8_t payload_len = frame[1] & 0x7F;
  size_t payload_offset = 2;

  if (payload_len == 126) {
    // 16 bit payload length
    if (frame.size() < 4) {
      return false;
    }
    payload_len = (static_cast<uint16_t>(frame[2]) << 8) |
                  static_cast<uint16_t>(frame[3]);
    payload_offset = 4;
  } else if (payload_len == 127) {
    // skip extended payloads
    return false;
  }

  bool maskBitSet = (frame[1] & 0x80) != 0;

  if (maskBitSet) {
    if (frame.size() < payload_offset + 4) {
      return false;
    }

    string masking_key = frame.substr(payload_offset, 4);

    payload_offset += 4;

    for (size_t i = 0; i < payload_len; ++i) {
      frame[payload_offset + i] =
          static_cast<uint8_t>(frame[payload_offset + i]) ^
          static_cast<uint8_t>(masking_key[i % masking_key.size()]);
    }
  }

  if (frame.size() < payload_offset + payload_len) {
    return false;
  }

  message.assign(frame, payload_offset, payload_len);

  return true;
}
