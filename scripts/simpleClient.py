import socket
import sys
import time
from struct import pack

host = 'localhost'
port = 8888

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((host, port))
response = sock.recv(1024)
import random
print(response)

def get_side(side):
    return bytes(side, 'ascii')

side = 'b'

while True:
    char = sys.stdin.read(1)
    if char == 'b':
        break
    # time.sleep(1)

    price = random.randrange(100, 1000)
    quantity = random.randrange(100, 200, 10)
    # price = 100
    # quantity = 100
    message = pack(
        'ciic',
        get_side(side),
        price,
        quantity,
        bytes(str(random.randrange(0, 9)), 'ascii')
    )

    sock.sendall(message)

    if side == 'b':
        side = 's'
    else:
        side = 'b'

    response = sock.recv(1024)
    print(response)

sock.close()

