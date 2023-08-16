#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_all.hpp>

#include "../../util/object_pool.h"

const char * pool_name = "/object_pool222";

TEST_CASE("Basic allocation test.") {
  MMapObjectPool<int> allocator(10, pool_name, true);

  int * a = allocator.allocate();
  REQUIRE(allocator.num_obj_stored() == 1);

  allocator.free(a);
  REQUIRE(allocator.num_obj_stored() == 0);

  allocator.cleanup();
};

TEST_CASE("Allocate and free randomly") {
  MMapObjectPool<int> allocator(10, "pool_name", true);

  int * a = allocator.allocate();
  REQUIRE(allocator.num_obj_stored() == 1);
  int * b = allocator.allocate();
  REQUIRE(allocator.num_obj_stored() == 2);

  allocator.free(b);
  REQUIRE(allocator.num_obj_stored() == 1);
  REQUIRE(allocator.num_random_free_spaces() == 1);

  allocator.free(a);
  REQUIRE(allocator.num_obj_stored() == 0);
  REQUIRE(allocator.num_random_free_spaces() == 2);

  // Finally make two allocations to see if 
  a = allocator.allocate();
  b = allocator.allocate();
  REQUIRE(allocator.num_obj_stored() == 2);
  REQUIRE(allocator.num_random_free_spaces() == 0);

  // Now that we've allocated all random free spacers we should be
  // using fresh memory locations
  a = allocator.allocate();
  REQUIRE(allocator.num_random_free_spaces() == 0);
  REQUIRE(allocator.num_obj_stored() == 3);

  allocator.cleanup();
};

TEST_CASE("Crash if we allocate more than we allow") {
  MMapObjectPool<int> allocator(1, "pool_name2", true);
  allocator.allocate();
  REQUIRE_THROWS(allocator.allocate());

  allocator.cleanup();
};

