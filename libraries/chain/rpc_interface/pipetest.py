import time
import sys
from pipe import PipeClient, PipeServer

MAX_SIZE = 1024
def start_server():
    print('start server.')
    s = PipeServer('test')
    while True:
        r = s.read(MAX_SIZE)
        print(r)
        s.write(r)

def start_client(count):
    print('start client')
    c = PipeClient('test')
    msg = bytes(MAX_SIZE)
    start = time.time()
    for i in range(count):
        c.write(msg)
        ret = c.read(MAX_SIZE)
        assert msg == ret
    duration = time.time() - start
    print(count/duration, ' msg/s')

if __name__ == '__main__':
    if len(sys.argv) >= 2 and sys.argv[1] == '1':
        start_server()
    else:
        start_client(10000)


