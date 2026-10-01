import socket
import threading

s = socket.socket()
s.connect(("192.168.4.1", 23))
print("connected")

def reader():
    try:
        while True:
            data = s.recv(1024)
            if not data:
                break
            print(data.decode(), end="")
    except OSError:
        pass
    print("\nlink lost")

threading.Thread(target=reader, daemon=True).start()

while True:
    cmd = input("> ")
    s.send((cmd + "\n").encode())