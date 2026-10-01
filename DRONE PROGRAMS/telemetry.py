import socket

s = socket.socket()
s.connect(("192.168.4.1", 24))
print("telemetry connected")

while True:
    data = s.recv(1024)
    if not data:
        break
    print(data.decode(), end="")