import socket, threading, json

BUFFER_SIZE = 32768


def create_dummy_state():
    state = dict()
    apple_location = {"x": 0.0, "y": 0.0}
    players = []
    for i in range(5):
        snake = {"id": i, "coords": [{"x": 1.0, "y": 2.0}]}
        players.append(snake)
    state["apple_location"] = apple_location
    state["players"] = players
    return state


state = create_dummy_state()


def listener(sock):
    while True:
        data = sock.recv(BUFFER_SIZE)
        print("Received:", data.decode())


def sender(sock):
    while True:
        packet = f"PUSH\nauth_token\n{json.dumps(state)}"
        sock.sendto(packet.encode(), ("127.0.0.1", 8000))


def client():
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
        listener_thread = threading.Thread(target=listener, args=(sock,))
        sender_thread = threading.Thread(target=sender, args=(sock,))

        listener_thread.start()
        sender_thread.start()

        listener_thread.join()
        sender_thread.join()


client()
