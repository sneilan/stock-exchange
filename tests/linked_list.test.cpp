#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>

#include "../src/util/linked_list.h"

TEST_CASE("Linked list removes data") {
  DoublyLinkedList<int> list;
  list.push_back(5);
  list.push_back(6);
  list.push_back(7);

  REQUIRE(list.get_front()->data == 5);
  REQUIRE(list.get_back()->data == 7);

  list.pop_front();
  REQUIRE(list.get_front()->data == 6);

  list.push_back(7);
}
