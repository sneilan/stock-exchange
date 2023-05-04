CC = /usr/bin/clang++
CFLAGS = -std=c++20 -fcolor-diagnostics -fansi-escape-codes `pkg-config libzmq --cflags` `pkg-config libzmq --libs` -I./util -lzmq
EXECUTABLE = main

OBJS = eventstore.o ./order_book/book.o ./order_book/price_level.o ./order_book/order_book.o gateway.o main.o

TEST_EXECUTABLE = ./tests/run_test
TEST_OBJS = ./tests/linked_list.test.o ./tests/helpers.o ./tests/order_book/book.test.o ./tests/order_book/price_level.test.o ./tests/order_book/order_book.test.o ./tests/main.test.o
TEST_FLAGS = -std=c++20 -fcolor-diagnostics -fansi-escape-codes `pkg-config libzmq --cflags` `pkg-config libzmq --libs` `pkg-config catch2 --cflags` `pkg-config catch2 --libs` -I./util -lzmq

all: $(EXECUTABLE)

eventstore.o: eventstore.cpp
	$(CC) $(CFLAGS) -c eventstore.cpp

./order_book/order_book.o: ./order_book/order_book.cpp
	$(CC) $(CFLAGS) -c ./order_book/order_book.cpp -o ./order_book/order_book.o

./order_book/book.o: ./order_book/book.cpp
	$(CC) $(CFLAGS) -c ./order_book/book.cpp -o ./order_book/book.o

./order_book/price_level.o: ./order_book/price_level.cpp
	$(CC) $(CFLAGS) -c ./order_book/price_level.cpp -o ./order_book/price_level.o

gateway.o: gateway.cpp
	$(CC) $(CFLAGS) -c gateway.cpp

main.o: main.cpp
	$(CC) $(CFLAGS) -c main.cpp

$(EXECUTABLE): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(EXECUTABLE)

./tests/linked_list.test.o: ./tests/linked_list.test.cpp
	$(CC) $(TEST_FLAGS) -c ./tests/linked_list.test.cpp -o ./tests/linked_list.test.o

./tests/helpers.o: ./tests/helpers.cpp
	$(CC) $(TEST_FLAGS) -c ./tests/helpers.cpp -o ./tests/helpers.o

./tests/order_book/order_book.test.o: ./tests/order_book/order_book.test.cpp
	$(CC) $(TEST_FLAGS) -c ./tests/order_book/order_book.test.cpp -o ./tests/order_book/order_book.test.o

./tests/order_book/book.test.o: ./tests/order_book/book.test.cpp
	$(CC) $(TEST_FLAGS) -c ./tests/order_book/book.test.cpp -o ./tests/order_book/book.test.o

./tests/order_book/price_level.test.o: ./tests/order_book/price_level.test.cpp
	$(CC) $(TEST_FLAGS) -c ./tests/order_book/price_level.test.cpp -o ./tests/order_book/price_level.test.o

./tests/main.test.o: ./tests/main.test.cpp
	$(CC) $(TEST_FLAGS) -c ./tests/main.test.cpp -o ./tests/main.test.o

$(TEST_EXECUTABLE): $(TEST_OBJS)
	$(CC) $(TEST_FLAGS) $(TEST_OBJS) order_book.o -o $(TEST_EXECUTABLE)

test: $(TEST_EXECUTABLE)
	./$(TEST_EXECUTABLE)

./tests/socketTest: ./tests/socketTest.cpp
	$(CC) -std=c++20 -I./util ./tests/socketTest.cpp -o ./tests/socketTest

socketTest: ./tests/socketTest
	./tests/socketTest

clean:
	rm -f $(EXECUTABLE) $(OBJS) $(TEST_OBJS) $(TEST_EXECUTABLE)
