#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>
#include <spdlog/spdlog.h>
#include <sodium.h>

int main(int argc, char *argv[]) {
  spdlog::set_level(spdlog::level::debug);
  spdlog::set_pattern("%-5l %E %-16s%-4#%-21! %v");

  if (sodium_init() == -1) {
    // Initialization failed
    spdlog::critical("Could not initialize libsodium!");
    return -1;
  }

  return Catch::Session().run(argc, argv);
}
