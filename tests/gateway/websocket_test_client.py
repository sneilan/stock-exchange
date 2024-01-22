#!/usr/bin/python3

import websocket
import ssl

print('Connecting to websocket')

# websocket.enableTrace(True)
ws = websocket.WebSocket(sslopt={"cert_reqs": ssl.CERT_NONE})

ws.connect("wss://localhost", timeout=5)
ws.send("asdf")
print("Got from server", ws.recv())
ws.close()
