#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>

#include "../../util/disruptor.h"

const char *disruptor_pool_name = "/disruptor_pool";

#define CONSUMER_1 0
#define CONSUMER_2 1

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

  Consumer<TestStruct> consumer(10, disruptor_pool_name, CONSUMER_1);

  TestStruct *pulledItem = consumer.get();
  REQUIRE(pulledItem->a == 5);
  REQUIRE(pulledItem->b == 6);

  REQUIRE(consumer.get() == nullptr);
  // try again to make sure nothing crashes.
  REQUIRE(consumer.get() == nullptr);

  producer.cleanup();
  consumer.cleanup();
};

TEST_CASE("Disruptor Producer Overflow Test") {
  int slots = 4;
  Producer<TestStruct> producer(slots, disruptor_pool_name);
  Consumer<TestStruct> consumer(slots, disruptor_pool_name, CONSUMER_1);

  TestStruct item;
  item.a = 5;
  item.b = 6;

  producer.put(item); // put at position 0
  producer.put(item); // 1
  producer.put(item); // 2
  producer.put(item); // 3

  // Consumer is at position 0
  // But should be able to get all three items.
  // Consumer starts at position 0.
  REQUIRE(consumer.get() != nullptr);
  REQUIRE(consumer.get() != nullptr);
  REQUIRE(consumer.get() != nullptr);
  REQUIRE(consumer.get() != nullptr);

  // no more items.
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
  Consumer<TestStruct> consumer(slots, disruptor_pool_name, CONSUMER_1);

  REQUIRE(consumer.get() == nullptr);

  producer.cleanup();
  consumer.cleanup();
};

TEST_CASE("Disruptor Producer / Multiple Consumer") {
  int slots = 4;
  Producer<TestStruct> producer(slots, disruptor_pool_name);
  Consumer<TestStruct> consumer1(slots, disruptor_pool_name, CONSUMER_1);
  Consumer<TestStruct> consumer2(slots, disruptor_pool_name, CONSUMER_2);

  TestStruct item;
  item.a = 5;
  item.b = 6;

  REQUIRE(producer.put(item) == true);
  REQUIRE(consumer1.get() != nullptr);
  REQUIRE(consumer1.get() == nullptr);

  REQUIRE(consumer2.get() != nullptr);
  REQUIRE(consumer2.get() == nullptr);

  REQUIRE(producer.put(item) == true);

  REQUIRE(consumer2.get() != nullptr);
  REQUIRE(consumer2.get() == nullptr);

  producer.cleanup();
  consumer1.cleanup();
  consumer2.cleanup();
};
