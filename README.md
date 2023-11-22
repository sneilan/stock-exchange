![CI](https://github.com/sneilan/stock-exchange/actions/workflows/tests.yml/badge.svg)

# 💻 Personal C++ Low Latency Stock Exchange

This is a stock exchange that you can run on your laptop or desktop that will process 10's of thousands of trades per second.

I built this as a fun nerdy project to show off my skills. Check out my [linkedin](https://linkedin.com/in/seanneilan)

* Create markets by connecting some trading robots to the exchange.
* Simulate any kind of market anytime - even outside of normal trading hours
* Plug in stock or crypto data and test against that
* Test against slippage and network failures
* Allow trading robots to develop new market patterns and write software to detect them

It uses the same techniques and algorithms as [NASDAQ](https://martinfowler.com/articles/lmax.html) but unoptimized.

Compare to [LMAX exchange](https://lmax-exchange.github.io/disruptor/).

## What is an Exchange?

For reference, a stock exchange is a server that takes buy/sell orders from traders and matches them up. When you open up Robinhood on your phone,
robinhood takes your order to buy Gamestop and sends it to an exchange called NASDAQ. NASDAQ finds a trader willing to sell you Gamestop and then
Robinhood sends you a notification once that sale is complete. This works vice-versa for sales. If you want to sell that share of Gamestop, Robinhood
sends your request to sell Gamestop to NASDAQ. NASDAQ finds someone willing to buy your share of Gamestop and once someone buys your share, tells Robinhood
to tell you!

## Running the Exchange
```
git clone git@github.com:sneilan/stock-exchange.git stock-exchange
cd stock-exchange
docker-compose up
```

The exchange will start up on the default port 8888.

## Place Sample Trades
In a separate terminal, not inside of docker, run the following example Python 3 trading client.
Python 3 script will auto-connect to 0.0.0.0:8888 (exchange server).
Will place a random trade each time you press enter. You'll see output on the exchange.
```
cd scripts
python3 scripts/loadTest.py
```

Script also functions as a 

## Limitations
* Do not run on a public server (yet)
* Server loses trades on shutdown (in progress)
* No cancellations, user accounts, balances, wallets.

Honestly there's a lot of work to do but I hope this becomes the premier stock exchange that everyone uses for personal experiments.

## Protocol

The exchange is basic for now. You connect, place trades and recieve notifications about your trades.

### Connect

Open a connection to the server with a socket. Use this in python

```python
import socket

host = '0.0.0.0'
port = 8888
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((host, port))
response = sock.recv(1024)
print("Connected!")
```

### Authentication 

_or lack thereof_

There is no authentication currently. 

You will be assigned a User ID however on connection that is not told
to the client except on trade notifications. Your "user id" is the socket number.

The exchange supports up to 30 concurrent clients.
First user to connect will have user id 0, second 1 and so one. If User 1 disconnects and a new user reconnects,
the new user will have user id 1 also. It's not great but it works for a demo. You do not need to authenticate

### What is traded

Currently the exchange trades one unnamed symbol. The name of the symbol
is whatever you want. To trade multiple symbols, start up multiple exchanges.

### Balances

Trade balances are infinite. Wallets and balances will come later.

### Risk Controls

No risk controls at the moment. Place as many trades as you like.

### Sending a trade

After connecting, send a trade by submitting 9 bytes.
1. Byte 0: Char -  buy or sell with 'b' for buy and 's' for sell.
2. Bytes 1-4 (inclusive) - Price as a positive unsigned integer in pennies.
3. Bytes 5-9 (inclusive) - Quantity as a positive unsigned integer.

Here's an example of sending a buy order in Python for 500 shares at
$1.23 / share. Assuming sock is an open socket to the exchange.

```python
price = 123 # $1.23 in 1 hundred 23 pennies.
quantity = 500
side = 'b' # 's' for sell
message = pack(
    'cii',
    bytes(side, 'ascii'),
    price,
    quantity,
)
sock.sendall(message)
```

Server will immediately send a trade notification with the id
of the trade.

### Trade Notifications

As soon as you send a trade, the server will tell you
the trade is recieved with the ID of the trade. Each trade
notification is 21 bytes with the first 

## Current features
```
[x] Connect from internet
[x] Place trades
[x] Get trade fill notifications
[x] Easy install
```

## Test
```
docker compose run -it core /app/test
```

Will run all tests automatically.

## TODO
There's a lot (to do)[https://github.com/sneilan/stock-exchange/issues] in creating a low-latency stock exchange from the ground up.

```
[ ] Market data
[ ] Cancel trades
[ ] Authentication / Security
[ ] Journaling (save trades to database)
[ ] Simple trading client / GUI
```

