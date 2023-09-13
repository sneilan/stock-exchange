#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>

#include "../../util/disruptor.h"

const char *disruptor_pool_name = "/disruptor_pool";

struct TestStruct {
  int a;
  int b;
};

TEST_CASE("Disruptor Pool Basic Test") {
  Producer<TestStruct> producer(10, disruptor_pool_name);

  TestStruct item;
  item.a = 5;
  item.b = 6;

  producer.put(item);

  Consumer<TestStruct> consumer(10, disruptor_pool_name);

  TestStruct *pulledItem = consumer.get();
  REQUIRE(pulledItem->a == 5);
  REQUIRE(pulledItem->b == 6);

  REQUIRE(consumer.get() == nullptr);
  REQUIRE(consumer.get() == nullptr);

  producer.cleanup();
  consumer.cleanup();
};

// For now our disruptor will expect data.
TEST_CASE("Disruptor Pool Producer Overflow Test", "[!shouldfail]") {
  int slots = 4;
  Producer<TestStruct> producer(slots, disruptor_pool_name);

  TestStruct item;
  item.a = 5;
  item.b = 6;

  producer.put(item);
  producer.put(item);
  producer.put(item);
  producer.put(item);
  // producer should be at position 0

  Consumer<TestStruct> consumer(slots, disruptor_pool_name);

  // Consumer is also at position 0
  // But should be able to get all four items.
  REQUIRE(consumer.get() != nullptr);
  REQUIRE(consumer.get() != nullptr);
  REQUIRE(consumer.get() != nullptr);
  REQUIRE(consumer.get() != nullptr);

  REQUIRE(consumer.get() == nullptr);

  producer.cleanup();
  consumer.cleanup();
};
