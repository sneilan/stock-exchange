import socket
import sys
from struct import pack

host = 'localhost'
port = 8888

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((host, port))
response = sock.recv(1024)
print(response)

def get_side(side):
    return bytes(side, 'ascii')

side = 'b'

while True:
    price = 500
    quantity = 1
    message = pack(
        'ciic',
        get_side(side),
        price,
        quantity,
        bytes('0', 'ascii')
    )

    sock.sendall(message)

    if side == 'b':
        side = 's'
    else:
        side = 'b'

    response = sock.recv(1024)
    print(response)

    # char = sys.stdin.read(1)
    # if char == 'b':
    #     break

sock.close()

