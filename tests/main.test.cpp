#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>
#include <spdlog/spdlog.h>

int main(int argc, char *argv[]) {
  spdlog::set_level(spdlog::level::debug);
  spdlog::set_pattern("%E.%F %g::%!::%@ [%l] %v");

  return Catch::Session().run(argc, argv);
}
