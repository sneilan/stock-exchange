# C++ LMAX Stock Exchange

Implementation of LMAX algorithm from https://martinfowler.com/articles/lmax.html for matching stock orders to demonstrate my knowledge of C++, Linux, virtual memory, ring buffers, order matching algorithms and generally thinking about how processes can work concurrently without locks by leveraging larger memory pools.

## Install
```
docker build -t exchange .
```

## Run
```
docker run -p 8888:8888 exchange
```

## Place Trades
```
# In a separate terminal, not inside of docker, run the following example Python 3 trading client.
# Python 3 script will auto-connect to 0.0.0.0:8888 (exchange server).
# Will place a random trade each time you press enter. You'll see output on the exchange.
python3 scripts/loadTest.py
```

## Test
```
# Get the docker id from docker ps
docker exec -it <id> /bin/bash
./test
```

## Notes to Self
Check out https://github.com/upandey3/MatchingEngine
