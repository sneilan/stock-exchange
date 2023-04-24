/usr/bin/clang++ -fcolor-diagnostics -fansi-escape-codes `pkg-config libzmq --cflags` `pkg-config libzmq --libs` -lzmq -c ./gateway.cpp
/usr/bin/clang++ -fcolor-diagnostics -fansi-escape-codes `pkg-config libzmq --cflags` `pkg-config libzmq --libs` -lzmq -c ./eventstore.cpp
/usr/bin/clang++ -fcolor-diagnostics -fansi-escape-codes `pkg-config libzmq --cflags` `pkg-config libzmq --libs` -lzmq -c ./orderBook.cpp
/usr/bin/clang++ -fcolor-diagnostics -fansi-escape-codes `pkg-config libzmq --cflags` `pkg-config libzmq --libs` -lzmq -c ./main.cpp

clang++ -fcolor-diagnostics -fansi-escape-codes `pkg-config libzmq --cflags` `pkg-config libzmq --libs` -lzmq eventstore.o orderBook.o gateway.o main.o -o main
