from base import  MyBinaryProtocol, MyBinaryProtocolFactory, IPCSocket, IPCServer
from threading import Thread
import time
import ipc

import logging
logging.basicConfig(format='%(lineno)d %(module)s :%(message)s', level=logging.INFO)

ipc.remove(b'server1')
ipc.remove(b'server2')
ipc.remove(b'client1')
ipc.remove(b'client2')

client = None
server = None
def start_client():
    global client
    client = IPCSocket(name=b'1')

    def _read():
        while True:
            ret = client.read(100)
            logging.info("read from client: ".format(ret))
            time.sleep(3.0)

    def _write():
        while True:
            text = b'write from client'
            logging.info(text)
            client.write(text)
            time.sleep(3.0)

    c = Thread(target=_read)
    s = Thread(target=_write)
    c.start()
    s.start()

    while True:
        time.sleep(3.0)
        
def start_server():
    global server
    server = IPCServer(b'1')
    
    server.listen()
    while True:
        client = server.accept()
        if not client:
            continue
        print('client connected', client)

        def _read():
            while True:
                ret = client.read(100)
                logging.info("read from server: {0}".format(ret))
                time.sleep(3.0)

        def _write():
            while True:
                logging.info(client.write(b'write from server'))
                time.sleep(3.0)
    
        c = Thread(target=_read)
        s = Thread(target=_write)
        c.start()
        s.start()

        while True:
            time.sleep(3.0)

def t():
    c = Thread(target=start_client)
    s = Thread(target=start_server)
    c.start()
    s.start()


def test_server():
    ipc.remove(b'1')
    ipc.remove(b'2')
    p1 = ipc.create(b'1', 100, 100)

    while True:
        time.sleep(1.0)
        p2 = ipc.open(b'2')
        if not p2:
            continue
        print('client connected.')
        while True:
            r = ipc.receive(p2, 100)
            if r == b'quit':
                break
            ipc.send(p1, r)

def test_client(count):
    p2 = ipc.create(b'2', 100, 100)
    while True:
        p1 = ipc.open(b'1')
        if p1:
            break
        time.sleep(1.0)
    print('server connected.')
    msg = b'hello,world'
    import time
    start = time.time()
    for i in range(count):
        ipc.send(p2, msg)
        r = ipc.receive(p1, 100)
        assert msg == r
    print(count/(time.time()-start))
    ipc.send(p2, b'quit')
    ipc.remove(b'2')




