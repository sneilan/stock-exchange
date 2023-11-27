import socket
import sys
import time
from struct import pack, unpack
import random
import threading


class Client:
    port = None
    host = None

    def log(self, msg):
        print(msg)

    def connect(self):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect((self.host, self.port))
        response = self.sock.recv(1024)
        self.log(response)

    def disconnect(self):
        self.sock.close()

    def listener(self):
        raise NotImplementedError()

    def start_listener(self):
        self.thread = threading.Thread(target=self.listener)
        self.thread.start()

    def stop_listener(self):
        raise NotImplementedError()


class MarketClient(Client):
    host = '0.0.0.0'
    port = 8889

    def listener(self):
        while True:
            data = self.sock.recv(16)
            print(data)
            print(len(data))
            if not data:
                return

            # c is char
            # Q is unsigned long long
            # i is 4 byte integer
            # x is 1 byte padding.
            format_string = 'BcxxiQ'
            unpacked_data = unpack(format_string, data)

            version = unpacked_data[0]
            msg_type = unpacked_data[1].decode()
            val = unpacked_data[2]
            time_ms = unpacked_data[3]

            self.handle_notification(msg_type, val, time_ms)

    def handle_notification(self, msg_type: str, val: int, time_ms: int):
        self.log({'msg_type': msg_type, 'val': val, 'time_ms': time_ms})


class TradingClient(Client):
    host = '0.0.0.0'
    port = 8888

    type_to_msg = {
        'u': 'updated',
        'f': 'filled',
        'r': 'recieved'
    }

    def __init__(self):
        pass

    def trade(self, price: int, quantity: int, side: str):
        assert side in ['b', 's'], 'Side must be b or s for buy and sell'

        message = pack(
            'cii',
            bytes(side, 'ascii'),
            price,
            quantity,
        )

        self.sock.sendall(message)

    def listener(self):
        while True:
            data = self.sock.recv(21)
            if not data:
                return

            # c is char
            # Q is unsigned long long
            # i is 4 byte integer
            format_string = 'Qiii'
            unpacked_data = unpack(format_string, data[1:])
            msg_type = chr(data[0])
            message = self.type_to_msg[msg_type]

            id = unpacked_data[0]
            quantity = unpacked_data[1]
            filled_quantity = unpacked_data[2]
            client_id = unpacked_data[3]

            self.handle_notification(id, quantity, filled_quantity, client_id)

    def handle_notification(self, id, quantity, filled_quantity, client_id):
        self.log({'id': id, 'quantity': quantity, 'filled_quantity': filled_quantity, 'client_id': client_id})


client = TradingClient()
client.connect()
client.start_listener()

mkt = MarketClient()
mkt.connect()
mkt.start_listener()

while True:
    char = sys.stdin.read(1)
    if char == 'b':
        break
    time.sleep(.1)
    print("Placing trade.")
    price = random.randint(101, 999)
    quantity = random.randint(1, 10)
    side = 'b' if random.randint(0, 1) == 0 else 's'
    client.trade(price, quantity, side)

