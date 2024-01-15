#!/usr/bin/python3

import websocket
import ssl

print("Connecting to websocket")
try:
    ws = websocket.create_connection("wss://localhost", sslopt={"cert_reqs": ssl.CERT_NONE})
    print("Connected to websocket")
except websocket.WebSocketConnectionClosedException:
    pass
