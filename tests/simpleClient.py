import zmq

context = zmq.Context()
socket = context.socket(zmq.REQ)
socket.connect("tcp://localhost:5555")

while True:
    # Client id 9 (allowed ids 0-9)
    # for limit price 55.55 (prices in format of xx.xx)
    # order side b = buy (s = sell)
    socket.send_string('955.55b')
    response = socket.recv()
    print(f"Received response: {response.decode()}")
