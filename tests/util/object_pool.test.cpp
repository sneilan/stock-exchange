#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>

#include "../../src/util/object_pool.h"

TEST_CASE("Basic object pool allocation test") {
  // Rest of object pool copied from mmap object pool.
  // Could probably subclass them so I don't have to write as many tests.
  ObjectPool<int> allocator(10);

  int *a = allocator.allocate();
  REQUIRE(allocator.num_obj_stored() == 1);

  allocator.del(a);
  REQUIRE(allocator.num_obj_stored() == 0);

  allocator.cleanup();
};

TEST_CASE("Malloc understanding test") {
  int * a = (int*)malloc(sizeof(int) * 5);
  int ** b = (int**)malloc(sizeof(int*) * 5);
  free(a);
  free(b);
}
