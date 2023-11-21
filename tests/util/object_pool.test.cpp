#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>

#include "../../src/util/object_pool.h"

const char *pool_name = "/object_pool";

TEST_CASE("Basic allocation test.") {
  MMapObjectPool<int> allocator(10, pool_name, IS_CONTROLLER);

  int *a = allocator.allocate();
  REQUIRE(allocator.num_obj_stored() == 1);

  allocator.free(a);
  REQUIRE(allocator.num_obj_stored() == 0);

  allocator.cleanup();
};

TEST_CASE("Allocate and free randomly") {
  MMapObjectPool<int> allocator(10, pool_name, IS_CONTROLLER);

  int *a = allocator.allocate();
  REQUIRE(allocator.num_obj_stored() == 1);
  int *b = allocator.allocate();
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
  MMapObjectPool<int> allocator(1, pool_name, true);
  allocator.allocate();
  REQUIRE_THROWS(allocator.allocate());

  allocator.cleanup();
};

TEST_CASE("Do offsets and memory locations line up") {
  MMapObjectPool<int> allocator(10, pool_name, IS_CONTROLLER);
  int *a = allocator.allocate();
  REQUIRE(allocator.pointer_to_offset(a) == 0);

  allocator.allocate();
  int *b = allocator.allocate();
  REQUIRE(allocator.pointer_to_offset(b) == 2);
  REQUIRE(allocator.offset_to_pointer(2) == b);

  allocator.cleanup();
}

TEST_CASE("Can processes share object pool") {
  MMapObjectPool<int> allocator(10, pool_name, IS_CONTROLLER);
  // allocate one object so we can see an offset of at least one.
  allocator.allocate();
  int *a = allocator.allocate();
  int a_offset = allocator.pointer_to_offset(a);

  *a = 5;

  pid_t c_pid = fork();
  if (c_pid > 0) {
    // parent. Will continue testing.
    REQUIRE(allocator.pointer_to_offset(a) == 1);
    int status;
    waitpid(c_pid, &status, 0);
  } else {
    // child
    MMapObjectPool<int> client_allocator(10, pool_name, IS_CLIENT);
    int *b = client_allocator.offset_to_pointer(a_offset);
    REQUIRE(*b == 5);
    REQUIRE(client_allocator.pointer_to_offset(b) == 1);
    client_allocator.cleanup();
    exit(0);
  }
};
