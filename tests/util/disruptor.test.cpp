#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>

#include "../../util/disruptor.h"

const char *disruptor_pool_name = "/disruptor_pool";

struct TestStruct {
  int a;
  int b;
};

TEST_CASE("Disruptor Basic Test") {
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

TEST_CASE("Disruptor Producer Overflow Test") {
  int slots = 4;
  Producer<TestStruct> producer(slots, disruptor_pool_name);
  Consumer<TestStruct> consumer(slots, disruptor_pool_name);

  TestStruct item;
  item.a = 5;
  item.b = 6;

  producer.put(item); // put at position 0
  producer.put(item); // 1
  producer.put(item); // 2
  // producer will have an overflow here because consumer is at position 0 and 4%4 = 0.
  REQUIRE(producer.put(item) == false);
  // producer should remain at position 3

  // Consumer is at position 0
  // But should be able to get all three items.
  // Consumer starts at position 0.
  REQUIRE(consumer.get() != nullptr);
  REQUIRE(consumer.get() != nullptr);
  REQUIRE(consumer.get() != nullptr);

  // Then no more items.
  REQUIRE(consumer.get() == nullptr);

  // consumer has consumed everything. should be able to put more stuff in.
  REQUIRE(producer.put(item) == true);
  REQUIRE(producer.put(item) == true);

  producer.cleanup();
  consumer.cleanup();
};

TEST_CASE("Disruptor Consumer Overflow Test") {
  int slots = 4;
  Producer<TestStruct> producer(slots, disruptor_pool_name);
  Consumer<TestStruct> consumer(slots, disruptor_pool_name);

  REQUIRE(consumer.get() == nullptr);

  producer.cleanup();
  consumer.cleanup();
};

TEST_CASE("Disruptor Producer / Consumer Chase Test") {
  int slots = 4;
  Producer<TestStruct> producer(slots, disruptor_pool_name);
  Consumer<TestStruct> consumer(slots, disruptor_pool_name);

  TestStruct item;
  item.a = 5;
  item.b = 6;

  REQUIRE(producer.put(item) == true);
  REQUIRE(consumer.get() != nullptr);
  REQUIRE(consumer.get() == nullptr);

  REQUIRE(producer.put(item) == true);
  REQUIRE(consumer.get() != nullptr);
  REQUIRE(consumer.get() == nullptr);

  REQUIRE(producer.put(item) == true);
  REQUIRE(consumer.get() != nullptr);
  REQUIRE(consumer.get() == nullptr);

  REQUIRE(producer.put(item) == true);
  REQUIRE(consumer.get() != nullptr);
  REQUIRE(consumer.get() == nullptr);

  REQUIRE(producer.put(item) == true);
  REQUIRE(consumer.get() != nullptr);
  REQUIRE(consumer.get() == nullptr);

  REQUIRE(producer.put(item) == true);
  REQUIRE(consumer.get() != nullptr);
  REQUIRE(consumer.get() == nullptr);

  REQUIRE(producer.put(item) == true);
  REQUIRE(consumer.get() != nullptr);
  REQUIRE(consumer.get() == nullptr);

  REQUIRE(producer.put(item) == true);
  REQUIRE(consumer.get() != nullptr);
  REQUIRE(consumer.get() == nullptr);

  REQUIRE(producer.put(item) == true);
  REQUIRE(consumer.get() != nullptr);
  REQUIRE(consumer.get() == nullptr);

  REQUIRE(consumer.get() == nullptr);

  producer.cleanup();
  consumer.cleanup();
};
