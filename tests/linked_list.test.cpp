#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_all.hpp>

#include "../util/linked_list.h"
#include <iostream>


int add(int a, int b) {
    return a + b;
}

TEST_CASE("Addition works correctly") {
    REQUIRE(add(2, 2) == 4);
    REQUIRE(add(0, 0) == 0);
    REQUIRE(add(-1, 1) == 0);
}

int main(int argc, char* argv[]) {
    return Catch::Session().run(argc, argv);
}
