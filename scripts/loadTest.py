import socket
import sys
import time
from struct import pack, unpack
import random
import threading
# this is crap
from _thread import *

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

def listener(sock):
    while True:
        data = sock.recv(21)
        if data:
            # c is char
            # Q is unsigned long long
            # i is 4 byte integer
            format_string = 'Qiii'
            unpacked_data = unpack(format_string, data[1:])
            msg_type = chr(data[0])
            message = ''
            if msg_type == 'u':
                message = 'updated'
            if msg_type == 'f':
                message = 'filled'
            if msg_type == 'r':
                message = 'recieved'

            id = unpacked_data[0]
            quantity = unpacked_data[1]
            filled_quantity = unpacked_data[2]
            client_id = unpacked_data[3]

            print('id', id, 'message', message, 'quantity', quantity, 'filled_quantity', filled_quantity, 'client_id', client_id)


start_new_thread(listener, (sock,))

while True:
    # char = sys.stdin.read(1)
    # if char == 'b':
    #     break
    time.sleep(.1)

    price = random.randrange(100, 1000)
    quantity = random.randrange(100, 200, 10)
    message = pack(
        'cii',
        bytes(side, 'ascii'),
        price,
        quantity,
    )
    side_msg = 'buy' if side == 'b' else 'sell'
    print(f"Placing {side_msg} for quantity {quantity} at price {price}")

    sock.sendall(message)
    # time.sleep(100000)

    if side == 'b':
        side = 's'
    else:
        side = 'b'

    # response = sock.recv(1024)
    # print(response)

sock.close()

