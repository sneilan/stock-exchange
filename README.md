![CI](https://github.com/sneilan/stock-exchange/actions/workflows/tests.yml/badge.svg)

# 💻 Personal C++ Low Latency Stock Exchange

This is a stock exchange that you can run on your laptop or desktop that will process 10's of thousands of trades per second.

I built this as a fun nerdy project to show off my skills. Check out my [linkedin](https://linkedin.com/in/seanneilan)

* Create markets by connecting some trading robots to the exchange.
* Simulate any kind of market anytime - even outside of normal trading hours
* Plug in stock or crypto data and test against that
* Test against slippage and network failures
* Allow trading robots to develop new market patterns and write software to detect them
# Easy install

It uses the same techniques and algorithms as [NASDAQ](https://martinfowler.com/articles/lmax.html) but unoptimized.

Compare to [LMAX exchange](https://lmax-exchange.github.io/disruptor/).

## What is an Exchange?

A stock exchange is a server that takes buy/sell orders from traders and matches them up. When you open up Robinhood on your phone,
robinhood takes your order to buy Gamestop and sends it to an exchange called NASDAQ. NASDAQ finds a trader willing to sell you Gamestop and then
Robinhood sends you a notification once that sale is complete. This works vice-versa for sales. If you want to sell that share of Gamestop, Robinhood
sends your request to sell Gamestop to NASDAQ. NASDAQ finds someone willing to buy your share of Gamestop and once someone buys your share, tells Robinhood
to tell you!

## Running the Exchange

Clone with
```
git clone git@github.com:sneilan/stock-exchange.git stock-exchange
cd stock-exchange
```

Run with
```
docker compose up
```

The exchange will start up on the default port `8888`.

## Place Sample Trades
In a separate terminal, not inside of docker, run the following example Python 3 trading client.
Python 3 script will auto-connect to server at `0.0.0.0:8888`.
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

### Sending a trade

After connecting, send a trade by submitting 9 bytes.
1. Byte 0: Char -  buy or sell with 'b' for buy and 's' for sell.
2. Bytes 1-4 (inclusive 4 bytes) - Price as a positive unsigned integer in pennies.
3. Bytes 5-8 (inclusive 4 bytes) - Quantity as a positive unsigned integer.

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
notification is 21 bytes.
1. Byte 0: Char - Notification type.
'r' is 'recieved'
'u' is 'updated'
'f' is 'filled'
2. Bytes 1-8 (inclusive 8 bytes): Unsigned long long - trade id
3. Bytes 9-12 (inclusive 4 bytes): Unsigned integer - quantity
4. Bytes 13-16 (inclusive 4 bytes): Unsigned integer - filled quantity
5. Bytes 17-20 (inclusive 4 bytes): Unsigned integer - client id
Client id is not important but will tell you what integer 0-30 your "user id" is.

You will always get a recieved notification. The other two notifications to a trade are either updated
or filled.

Here's an example of recieving trade notifications in Python. It assumes that sock is a connected
socket to the server.

```python
from struct import unpack

msg_type_to_msg = {}
msg_type_to_msg['u'] = 'updated'
msg_type_to_msg['f'] = 'filled'
msg_type_to_msg['r'] = 'recieved'
while True:
    data = sock.recv(21)
    if data:
        # c is char
        # Q is unsigned long long
        # i is 4 byte integer
        # I originally tried to use 'cQiii' but unpack would not
        # parse the bytes correctly. This works for now.
        format_string = 'Qiii'
        unpacked_data = unpack(format_string, data[1:])
        msg_type = chr(data[0])
        message = msg_type_to_msg[msg_type]
        id = unpacked_data[0]
        quantity = unpacked_data[1]
        filled_quantity = unpacked_data[2]
        client_id = unpacked_data[3]

        print('id', id, 'message', message, 'quantity', quantity, 'filled_quantity', filled_quantity, 'client_id', client_id)
```

Check out scripts/loadTest.py for an example trading client.

You can paste these protocols into Chat GPT and produce trading frontends in your preferred language.

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

### Market Data

This is not implemented yet. This is a high priority on the roadmap.

### Getting Current Bid / Ask

Not implemented. Very high priority!

### Backups

Not implemented.

## Test
```
docker compose run -it core /app/test
```

Will run all tests automatically.

## TODO
There's a lot (to do)[https://github.com/sneilan/stock-exchange/issues] in creating a low-latency stock exchange from the ground up.

Check out the issue list.

## Contributing

Check out any of the tickets on the issues list or file a new one with a proposal!

