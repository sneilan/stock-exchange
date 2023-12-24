#include <openssl/pem.h>
#include <openssl/rand.h>
#include <stdio.h>

TEST_CASE("OpenSSL Test - Encrypting and Decrypting") {
  SSL_CTX *ctx = SSL_CTX_new(SSLv23_server_method());
  REQUIRE(SSL_CTX_use_certificate_file(ctx, getenv("PUBLIC_KEY"), SSL_FILETYPE_PEM) > 0);
  REQUIRE(SSL_CTX_use_PrivateKey_file(ctx, getenv("PRIVATE_KEY"), SSL_FILETYPE_PEM) > 0);

  SSL *ssl = SSL_new(ctx);
  SSL_shutdown(ssl);
  SSL_free(ssl);

  SSL_CTX_free(ctx);
}
