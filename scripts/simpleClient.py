import socket
import sys
import gateway.proto.incoming_order_pb2 as incoming_order

host = 'localhost'
port = 8888

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((host, port))

# send/receive data here.
message = incoming_order.IncomingOrder()
message.limitPrice = 100
message.side = "b"
message.clientId = 123
message.quantity = 10
# print(message.SerializeToString())

while True:
    char = sys.stdin.read(1)
    sock.sendall(message.SerializeToString())
    if char == 'b':
        break
    # response = sock.recv(1024)

sock.close()

