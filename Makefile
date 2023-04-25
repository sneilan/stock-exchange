CC = /usr/bin/clang++
CFLAGS = -fcolor-diagnostics -fansi-escape-codes `pkg-config libzmq --cflags` `pkg-config libzmq --libs` -lzmq
EXECUTABLE = main

OBJS = eventstore.o orderBook.o gateway.o main.o

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

clean:
	rm -f $(EXECUTABLE) $(OBJS)
