/usr/bin/clang++ -fcolor-diagnostics -fansi-escape-codes `pkg-config libzmq --cflags` `pkg-config libzmq --libs` -lzmq -c /Users/sneilan/Documents/exchange/gateway.cpp
/usr/bin/clang++ -fcolor-diagnostics -fansi-escape-codes `pkg-config libzmq --cflags` `pkg-config libzmq --libs` -lzmq -c /Users/sneilan/Documents/exchange/eventstore.cpp
/usr/bin/clang++ -fcolor-diagnostics -fansi-escape-codes `pkg-config libzmq --cflags` `pkg-config libzmq --libs` -lzmq -c /Users/sneilan/Documents/exchange/main.cpp
clang++ -fcolor-diagnostics -fansi-escape-codes `pkg-config libzmq --cflags` `pkg-config libzmq --libs` -lzmq eventstore.o gateway.o main.o -o main
