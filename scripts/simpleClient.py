import socket
import sys
import gateway.proto.incoming_order_pb2 as incoming_order

host = 'localhost'
port = 8888

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((host, port))
response = sock.recv(1024)
print(response)

# send/receive data here.
message = incoming_order.IncomingOrder()
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

