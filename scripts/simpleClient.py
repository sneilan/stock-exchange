import socket
import sys

host = 'localhost'
port = 8888

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((host, port))
response = sock.recv(1024)
print(response)

message = bytearray()
# buy
message.append()
# sell


    char side;
    // Stored in pennies
    // $10.05 = 1005
    PRICE limitPrice;
    int quantity;
    char clientId;

message.limitPrice = 100
message.side = "b"
message.clientId = 123
message.quantity = 10
# print(message.SerializeToString())

while True:
    sock.sendall(message.SerializeToString())
    if message.side == 'b':
        message.side = 's'
    else:
        message.side = 'b'
    response = sock.recv(1024)
    print(response)
    # char = sys.stdin.read(1)
    # if char == 'b':
    #     break

sock.close()

