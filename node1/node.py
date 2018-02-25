import socket

SERVER_IP="192.168.0.19"
SERVER_PORT=1234

def Connect():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((SERVER_IP, SERVER_PORT))
    if s is None:
        print("Could not connect :(")
        sys.exit(1)
    else:
        print("Connected")
    return s

#s=socket.socket
try:
    s = Connect()
    out = "Hello"
    #s.sendall(out)
    msg_in = s.recv(4096)
    print(msg_in)

except socket.error:
    print ("Exception")
finally:
    #s.close()
    print("Finally")
