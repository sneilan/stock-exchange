import websocket
import ssl

ws = websocket.create_connection("wss://localhost", sslopt={"cert_reqs": ssl.CERT_NONE})

print("Connected to websocket")

ws.close()

