import zmq

context = zmq.Context()
socket = context.socket(zmq.REQ)
socket.connect("tcp://localhost:5555")

while True:
    socket.send_string('955.55b')
    response = socket.recv()
    print(f"Received response: {response.decode()}")
