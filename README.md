# C++ LMAX Stock Exchange

Implementation of LMAX algorithm from https://martinfowler.com/articles/lmax.html for matching stock orders to demonstrate my knowledge of C++, Linux, virtual memory, ring buffers, order matching algorithms and generally thinking about how processes can work concurrently without locks by leveraging larger memory pools.

## Installation
```
docker build -t exchange .
```

## Running
```
docker run -p 8888:8888 exchange
python scripts/simpleClient.py # Will place a random trade each time you press enter.
```

## Tests
```
# Get the docker id from docker ps
docker exec -it <id> /bin/bash
./test
```

## Notes to Self
Check out https://github.com/upandey3/MatchingEngine

