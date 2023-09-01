import socket
import sys
import time
from struct import pack

host = '0.0.0.0'
port = 8888

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((host, port))
response = sock.recv(1024)
import random
print(response)

def get_side(side):
    return bytes(side, 'ascii')

side = 'b'
count = 0
NumOfSeconds=100

while True:
    char = sys.stdin.read(1)
    if char == 'b':
        break
    # time.sleep(.0001)

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
    count += 1
    print(float(count)/NumOfSeconds)

sock.close()

