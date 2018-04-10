# -*- coding: utf8 -*-
from struct import pack, unpack

from threading import Thread
from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol

#server side
from thrift.server import TServer
from idl import eoslib_service
from idl import ttypes

#client side
from idl.rpc_interface import Client
from idl.rpc_interface import Result

import time
try:
    import eoslib
except:
    pass

HOST = 'localhost'
APPLY_PORT = 8080
DB_PORT = 8081


'''
i32 db_store_i64( i64 scope, i64 table, i64 payer, i64 id, 5:binary buffer );
void db_update_i64( i32 itr, i64 payer, binary buffer );
void db_remove_i64( i32 itr );
Result db_get_i64( i32 itr );
Result db_next_i64( i32 itr);
Result db_previous_i64( i32 itr );
i32 db_find_i64( i64 code, i64 scope, i64 table, i64 id );
i32 db_lowerbound_i64( i64 code, i64 scope, i64 table, i64 id );
i32 db_upperbound_i64( i64 code, i64 scope, i64 table, i64 id );
i32 db_end_i64( i64 code, i64 scope, i64 table );
'''

class RequestHandler(object):

    def db_store_i64(self, scope: int, table: int, payer: int, id: int, buffer: bytes ):
        return eoslib.store_i64(scope, table, payer, id, buffer)

    def db_update_i64(self, itr: int, payer: int, buffer: bytes ):
        eoslib.update_i64(itr, payer, buffer)

    def db_remove_i64(self, itr: int ):
        eoslib.remove_i64(itr) 

    def db_get_i64(self, itr: int ):
        return b'hello, world' #eoslib.get_i64(itr)

    def db_next_i64(self, itr: int):
        itr_next, primary = eoslib.next_i64(itr)
        return Result(itr_next, primary)

    def db_previous_i64(self, itr: int ):
        itr_next, primary = eoslib.previous_i64(itr)
        return Result(itr_next, primary)

    def db_find_i64(self, code: int, scope: int, table: int, id: int ):
        return eoslib.find_i64(code, scope, table, id)
    
    def db_lowerbound_i64(self, code: int, scope: int, table: int, id: int ):
        return eoslib.lowerbound_i64(code, scope, table, id)
    
    def db_upperbound_i64(self, code: int, scope: int, table: int, id: int ):
        return eoslib.upperbound_i64(code, scope, table, id)

    def db_end_i64(self, code: int, scope: int, table: int ):
        return eoslib.end_i64(code, scope, table)

class DBServer(TServer.TServer, Thread):
    """Simple single-threaded server that just pumps around one transport."""

    def __init__(self, *args):
        Thread.__init__(self)
        TServer.TServer.__init__(self, *args)

    def run(self):
        self.serve()

    def serve(self):
        print('Starting the rpc server at', HOST,':', DB_PORT)
        self.serverTransport.listen()
        while True:
            client = self.serverTransport.accept()
            if not client:
                continue
            print('client connected', client)
            itrans = self.inputTransportFactory.getTransport(client)
            otrans = self.outputTransportFactory.getTransport(client)
            iprot = self.inputProtocolFactory.getProtocol(itrans)
            oprot = self.outputProtocolFactory.getProtocol(otrans)
            try:
                while True:
                    self.processor.process(iprot, oprot)
            except TTransport.TTransportException:
                pass
            except Exception as x:
                logger.exception(x)
            print('clinet disconnected')
            itrans.close()
            otrans.close()

class MyBinaryProtocol(TBinaryProtocol.TBinaryProtocol):
    def writeI64(self, i64):
        buff = pack("!Q", i64)
        self.trans.write(buff)

    def readI64(self):
        buff = self.trans.readAll(8)
        val, = unpack('!Q', buff)
        return val

def start():
    handler = RequestHandler()
    
    processor = eoslib_service.Processor(handler)
    transport = TSocket.TServerSocket(HOST, DB_PORT)
    tfactory = TTransport.TBufferedTransportFactory()
    pfactory = TBinaryProtocol.TBinaryProtocolFactory()
    
    rpcServer = DBServer(processor,transport, tfactory, pfactory)
    rpcServer.start()

client = None
def open_client():
    global client
    tsocket = TSocket.TSocket(HOST, APPLY_PORT)
    transport = TTransport.TBufferedTransport(tsocket)
    protocol = MyBinaryProtocol(transport)
    client = Client(protocol)
    transport.open()

def apply(account, action, code):
    global client
    if not client:
        open_client()
    ret = None
#    account = int.to_bytes(account, 8, 'little')
    try:
        ret = client.apply(account, action, code)
    except Exception as e:
        print(e)
        #try again
        open_client()
        ret = client.apply(account, action, code)

    print('done!', ret)
    return ret

if __name__ == '__main__':
    apply(1234, 234, b'hello,mike')

