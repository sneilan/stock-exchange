#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>
#include <spdlog/spdlog.h>
#include <sodium.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <stdexcept>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  spdlog::set_level(spdlog::level::debug);
  spdlog::set_pattern("%-5l %E %-16s%-4#%-21! %v");

  if (sodium_init() == -1) {
    // Initialization failed
    spdlog::critical("Could not initialize libsodium!");
    return -1;
  }

  OpenSSL_add_all_algorithms();

  int rc = system("/app/scripts/operations/create_local_ssl_keys.sh");
  if (rc != 0) {
    throw std::runtime_error("Could not generate public/private keys");
  }

  return Catch::Session().run(argc, argv);
}
