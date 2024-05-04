#!/usr/bin/python3

import websocket
import ssl
import json

print('Connecting to websocket')

websocket.enableTrace(True)
def mask_key(l):
    return b'4544'
    # return 'a' * l

ws = websocket.WebSocket(sslopt={"cert_reqs": ssl.CERT_NONE})
# ws.set_mask_key(mask_key)

ws.connect("wss://localhost", timeout=5)
message = {'msg': 'Hello world from client!'}
ws.send(json.dumps(message))
print("Got from server", ws.recv())
ws.close()
