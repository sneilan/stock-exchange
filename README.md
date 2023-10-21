# 💻 Personal C++ Low Latency Stock Exchange 🤑

This is a stock exchange that you can run on your laptop or desktop that will process 10's of thousands of trades per second.
It's better for testing trading strategies than backtesting because you can try your ideas out against an actual market that you create locally.
* Create markets by connecting some trading robots to the exchange.
* Simulate any kind of market anytime - even outside of normal trading hours!
* Plug in stock or crypto data and test against that
* Test against slippage and network failures
* Allow trading robots to develop new market patterns and write software to detect them!

It uses the same [techniques and algorithms as NASDAQ but on a smaller scale](https://martinfowler.com/articles/lmax.html).

## What is an Exchange?

For reference, a stock exchange is a server that takes buy/sell orders from traders and matches them up. When you open up Robinhood on your phone,
robinhood takes your order to buy Gamestop and sends it to an exchange called NASDAQ. NASDAQ finds a trader willing to sell you Gamestop and then
Robinhood sends you a notification once that sale is complete. This works vice-versa for sales. If you want to sell that share of Gamestop, Robinhood
sends your request to sell Gamestop to NASDAQ. NASDAQ finds someone willing to buy your share of Gamestop and once someone buys your share, tells Robinhood
to tell you!

## How to Install
```
git clone git@github.com:sneilan/stock-exchange.git stock-exchange
cd stock-exchange
docker build -t exchange .
```

## Running the Exchange
```
cd stock-exchange
docker run -it -v $PWD:/app -p 8888:8888 exchange
```

## Current features
```
[x] Connect from internet
[x] Place trades
[x] Get trade fill notifications
[x] Easy install
```

## Place Trades
In a separate terminal, not inside of docker, run the following example Python 3 trading client.
Python 3 script will auto-connect to 0.0.0.0:8888 (exchange server).
Will place a random trade each time you press enter. You'll see output on the exchange.
```
python3 scripts/loadTest.py
```

## Test
Test with
```
docker run -it -v $PWD:/app exchange /app/test
```
Will run all tests automatically.

## TODO
There's a lot to do in creating a low-latency stock exchange from the ground up that's fast and 
```
[ ] Market data
[ ] Cancel trades
[ ] Authentication / Security
[ ] Journaling (save trades to database)
[ ] Simple trading client / GUI
```
