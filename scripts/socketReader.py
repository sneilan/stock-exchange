import socket

# Create a TCP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Connect to a remote server
server_address = ('localhost', 8888)
sock.connect(server_address)

# Read data from the socket
while True:
    data = sock.recv(1024)
    if not data:
        break
    print(data.decode('utf-8'))

# Clean up
sock.close()
