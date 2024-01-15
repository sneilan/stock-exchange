#!/usr/bin/python3

import websocket
import ssl

print('Connecting to websocket')

websocket.enableTrace(True)
ws = websocket.WebSocket(sslopt={"cert_reqs": ssl.CERT_NONE})

# print("Connecting to websocket")
ws = ws.connect("wss://localhost", timeout=5)
# ws.recv()
# ws.send("asdf")
ws.close()
print("Connected to websocket")
