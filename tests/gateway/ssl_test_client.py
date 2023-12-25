import socket
import ssl
import os

hostname = 'localhost'
context = ssl.create_default_context()

with socket.create_connection((hostname, int(os.getenv("GATEWAY_PORT", 443)))) as sock:
    with context.wrap_socket(sock, server_hostname=hostname) as ssock:
        print(ssock.version())

