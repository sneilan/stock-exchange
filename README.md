# C++ LMAX Stock Exchange

Implementation of LMAX algorithm from https://martinfowler.com/articles/lmax.html for matching stock orders to demonstrate my knowledge of C++, Linux, virtual memory, ring buffers, order matching algorithms and generally thinking about how processes can work concurrently without locks by leveraging larger memory pools.

## Running
```
make
./main
```

## Tests
```
brew install catch2 # or apt-get / yum / whatever for your platform.
make test
```
