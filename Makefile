CC = /usr/bin/clang++
CFLAGS = -fcolor-diagnostics -fansi-escape-codes `pkg-config libzmq --cflags` `pkg-config libzmq --libs` -lzmq
EXECUTABLE = main

OBJS = eventstore.o orderBook.o gateway.o main.o

TEST_EXECUTABLE = ./tests/run_test
TEST_OBJS = ./tests/linked_list.test.o
TEST_FLAGS = -std=c++14 `pkg-config catch2 --cflags` `pkg-config catch2 --libs`

all: $(EXECUTABLE)

eventstore.o: eventstore.cpp
	$(CC) $(CFLAGS) -c eventstore.cpp

orderBook.o: orderBook.cpp
	$(CC) $(CFLAGS) -c orderBook.cpp

gateway.o: gateway.cpp
	$(CC) $(CFLAGS) -c gateway.cpp

main.o: main.cpp
	$(CC) $(CFLAGS) -c main.cpp

$(EXECUTABLE): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(EXECUTABLE)

./tests/linked_list.test.o: ./tests/linked_list.test.cpp
	$(CC) $(TEST_FLAGS) -c ./tests/linked_list.test.cpp -o ./tests/linked_list.test.o

$(TEST_EXECUTABLE): $(TEST_OBJS)
	$(CC) $(TEST_FLAGS) $(TEST_OBJS) -o $(TEST_EXECUTABLE)

test: $(TEST_EXECUTABLE)
	./$(TEST_EXECUTABLE)

clean:
	rm -f $(EXECUTABLE) $(OBJS) $(TEST_OBJS) $(TEST_EXECUTABLE)
