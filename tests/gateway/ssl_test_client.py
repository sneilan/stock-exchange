#!/usr/bin/python3

import socket
import ssl
import os

hostname = 'localhost'
context = ssl.create_default_context()
context.check_hostname = False
context.verify_mode = ssl.CERT_NONE

with socket.create_connection((hostname, int(os.getenv("GATEWAY_PORT", 443)))) as sock:
    with context.wrap_socket(sock, server_hostname=hostname) as ssock:
        print(ssock.version())
