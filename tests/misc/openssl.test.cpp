#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <spdlog/spdlog.h>
#include <stdio.h>
#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>


TEST_CASE("OpenSSL Test - Encrypting and Decrypting") {
  // https://www.openssl.org/docs/manmaster/man3/SSL_CTX_use_certificate_file.html
  SSL_CTX *ctx = SSL_CTX_new(SSLv23_server_method());
  REQUIRE(SSL_CTX_use_certificate_file(ctx, getenv("CERTIFICATE"), SSL_FILETYPE_PEM) > 0);
  REQUIRE(SSL_CTX_use_PrivateKey_file(ctx, getenv("PRIVATE_KEY"), SSL_FILETYPE_PEM) > 0);

  SSL *ssl = SSL_new(ctx);
  SSL_shutdown(ssl);
  SSL_free(ssl);

  SSL_CTX_free(ctx);
}
