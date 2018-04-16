import time
from struct import pack, unpack
from thrift.protocol import TBinaryProtocol

import logging
logging.basicConfig(format='%(lineno)d %(module)s :%(message)s', level=logging.ERROR)

import ipc

MAX_MSG_SIZE=128

class MyBinaryProtocol(TBinaryProtocol.TBinaryProtocol):
    def writeI64(self, i64):
        buff = pack("!Q", i64)
        self.trans.write(buff)

    def readI64(self):
        buff = self.trans.readAll(8)
        val, = unpack('!Q', buff)
        return val

class MyBinaryProtocolFactory(TBinaryProtocol.TBinaryProtocolFactory):
    def getProtocol(self, trans):
        prot = MyBinaryProtocol(trans, self.strictRead, self.strictWrite,
                               string_length_limit=self.string_length_limit,
                               container_length_limit=self.container_length_limit)
        return prot

class IPCSocket(object):
    
    def __init__(self, read  = None, write = None, name = None):
        if name:
            self.ipc_write = ipc.create(b'client'+name, 100, MAX_MSG_SIZE)
            while True:
                self.ipc_read = ipc.open(b'server'+name) # ipc for read create by server
                if self.ipc_read:
                    break
                logging.info('waitting for server...{0}'.format(b'server'+name))
                time.sleep(3.0)
            ipc.send(self.ipc_write, b'hello')
            ret = ipc.receive(self.ipc_read, MAX_MSG_SIZE)
            print(ret)
            assert ret == b'ok'
        else:
            self.ipc_read = read
            self.ipc_write = write
            
        self.cache = bytearray()

    def setHandle(self, h):
        pass

    def isOpen(self):
        return self.ipc_read is not None

    def setTimeout(self, ms):
        pass

    def open(self):
        pass

    def read(self, sz):
        ret = None
        if sz <= len(self.cache):
            ret = self.cache[:sz]
            self.cache = self.cache[sz:]
        else:
            self.cache += ipc.receive(self.ipc_read, MAX_MSG_SIZE)
            ret = self.cache[:sz]
            self.cache = self.cache[sz:]
#        logging.info("sz: {0}, len(ret): {1} read: {2}".format(sz, len(ret), ret))
        return bytes(ret)

    def readAll_(self, sz):
        buff = b''
        have = 0
        while (have < sz):
            chunk = self.read(sz - have)
            chunkLen = len(chunk)
            have += chunkLen
            buff += chunk

            if chunkLen == 0:
                raise EOFError()

        return buff

    def readAll(self, sz):
#        logging.info("readAll: {0}".format(sz))
        return self.read(sz)

    def write(self, buff):
#        logging.info("write:{0}".format(buff))
        ipc.send(self.ipc_write, buff)

    def flush(self):
        pass
    
    def close(self):
        pass

class IPCServer(object):
    def __init__(self,name):
        self.name = name
        #ipc for write to client
        self.ipc_write = ipc.create(b'server'+name, 100, MAX_MSG_SIZE)
        self.ipc_read = None

    def listen(self):
        while True:
            self.ipc_read = ipc.open(b'client'+self.name) #ipc for read from client
            if self.ipc_read:
                break
            time.sleep(3.0)
        ret = ipc.receive(self.ipc_read, MAX_MSG_SIZE)
        logging.info('++++++++listen: ipc.receive {0}'.format(ret))
        if ret == b'hello':
            ipc.send(self.ipc_write, b'ok')
        else:
            assert 0
    def accept(self):
        return IPCSocket(self.ipc_read, self.ipc_write)

