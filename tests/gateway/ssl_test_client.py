#!/usr/bin/python3

import socket
import ssl
import os
import sys

hostname = 'localhost'
context = ssl.create_default_context()
context.check_hostname = False
context.verify_mode = ssl.CERT_NONE

with socket.create_connection((hostname, int(os.getenv("GATEWAY_PORT", 443)))) as sock:
    with context.wrap_socket(sock, server_hostname=hostname) as ssock:
        message = "Hello world"

        ssock.send(message.encode('utf-8'))

        response = ssock.recv(1024)
        print("Server response:", response.decode('utf-8'))

        ssock.close()
