 #named pipe Server  
#encoding: utf-8  
#reference from from https://blog.csdn.net/mayao11/article/details/50618598
import os
import time  
from thrift.transport import TSocket
from thrift.server import TServer
from thrift.transport import TTransport

MAX_SIZE = 1024

class PipeClient(TTransport.TTransportBase):
    def __init__(self, name):
        self.write_path = "/tmp/server_in_{0}.pipe".format(name)
        self.read_path = "/tmp/server_out_{0}.pipe".format(name)
        self._write_fd = os.open(self.write_path, os.O_SYNC | os.O_CREAT | os.O_RDWR)
        self._read_fd = None

    def isOpen(self):
        return True

    def open(self):
        pass

    def write(self, buf):
        return os.write(self._write_fd, buf)

    def read(self, size):
        if not self._read_fd:
            self._read_fd = os.open(self.read_path, os.O_RDONLY)
        while True:
            ret = os.read(self._read_fd, size)
            if ret:
                return ret

    def flush(self):
        pass

    def close(self):
        os.close(self._read_fd)
        os.close(self._write_fd)

class PipeServer(TTransport.TTransportBase):
    def __init__(self, name):
        self.read_path = "/tmp/server_in_{0}.pipe".format(name)
        self.write_path = "/tmp/server_out_{0}.pipe".format(name)

        try:
            os.mkfifo(self.read_path)
            os.mkfifo(self.write_path)
        except OSError as e:
            pass

        self._read = os.open(self.read_path, os.O_RDONLY)
        self._write = os.open(self.write_path, os.O_SYNC | os.O_CREAT | os.O_RDWR)
        print('open finished.')

    def listen(self):
        while True:
            ret = self.read(MAX_SIZE)
            print('ret:', ret)
            if ret == b'hello':
                return True
            print('waitting...')
            time.sleep(1.0)

    def accept(self):
        return self

    def isOpen(self):
        return True

    def read(self, size):
        while True:
            ret = os.read(self._read, size)
            if ret:
                return ret

    def write(self, buf):
        return os.write(self._write, buf)

    def close(self):
        os.close(self._read)  
        os.close(self._write) 

