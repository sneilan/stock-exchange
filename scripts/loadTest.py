import socket
import sys
from struct import pack
import random

# Connects to exchange and asks the user to place an

print("This program connects to the exchange and places as many random trades as possible.")
print("Press enter to begin")
sys.stdin.read(1)

# Connect to exchange.
print("Connecting to exchange...")
host = '0.0.0.0'
port = 8888
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((host, port))
response = sock.recv(1024)
print("Connected!")

# Initial side.
side = 'b'

while True:
    char = sys.stdin.read(1)
    if char == 'b':
        break

    price = random.randrange(100, 1000)
    quantity = random.randrange(100, 200, 10)
    message = pack(
        'ciic',
        bytes(side, 'ascii'),
        price,
        quantity,
        bytes(str(random.randrange(0, 9)), 'ascii')
    )
    side_msg = 'buy' if side == 'b' else 'sell'
    print(f"Placing {side_msg} for quantity {quantity} at price {price}")

    sock.sendall(message)

    if side == 'b':
        side = 's'
    else:
        side = 'b'

    response = sock.recv(1024)
    print(response)

sock.close()

