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
# for ubuntu/debian/popos
sudo apt install libspdlog-dev
# install Catch2 from https://github.com/catchorg/Catch2/blob/devel/docs/cmake-integration.md#installing-catch2-from-git-repository
# (I use the git clone version)
cmake .
make
make test
```

## Notes to Self
Check out https://github.com/upandey3/MatchingEngine

