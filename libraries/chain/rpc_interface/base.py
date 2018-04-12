from struct import pack, unpack
from thrift.protocol import TBinaryProtocol

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

