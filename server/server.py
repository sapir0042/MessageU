import socket
import selectors
from protocol import handle_request

HOST = '127.0.0.1'

sel = selectors.DefaultSelector()


def accept(sock):
    conn, addr = sock.accept()  # Should be ready
    print('accepted', conn, 'from', addr)
    conn.setblocking(False)
    sel.register(conn, selectors.EVENT_READ, handle_request)


def server(port):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((HOST, port))
        s.listen(100)
        s.setblocking(False)
        sel.register(s, selectors.EVENT_READ, accept)

        print('server up')
        while True:
            events = sel.select()
            for key, mask in events:
                callback = key.data
                callback(key.fileobj)


if __name__ == '__main__':
    try:
        with open('port.info') as f:
            port = int(f.read())
            server(port)
    except FileNotFoundError:
        print("Error: file not found")
