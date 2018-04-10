# -*- coding: utf8 -*-
import imp
import logging
from struct import pack, unpack
from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol

#client side
from idl.eoslib_service import Client
from idl.eoslib_service import Result
import time


#//server side
from idl import eoslib_service
from idl import rpc_interface

from idl import ttypes
from thrift.server import TServer

import eoslib

HOST = 'localhost'
APPLY_PORT = 8080
DB_PORT = 8081

def n2s(value):
    charmap = ".12345abcdefghijklmnopqrstuvwxyz";
    
    name = []
    
    tmp = value;
    for i in range(13):
        if i == 0:
            c = charmap[tmp & 0x0f]
        else:
            c = charmap[tmp & 0x1f]

#        str[12-i] = c;
        name.append(c)
        if i == 0:
            tmp >>= 4
        else:
            tmp >>= 5
    name.reverse()
    return ''.join(name).rstrip('.')

class MyBinaryProtocol(TBinaryProtocol.TBinaryProtocol):
    def writeI64(self, i64):
        buff = pack("!Q", i64)
        self.trans.write(buff)

    def readI64(self):
        buff = self.trans.readAll(8)
        val, = unpack('!Q', buff)
        return val

class RequestHandler(object):
    def __init__(self):
        self.modules = {}
        self.client = self.set_client()

    def set_client(self):
        tsocket = TSocket.TSocket(HOST, DB_PORT)
        transport = TTransport.TBufferedTransport(tsocket)
        protocol = MyBinaryProtocol(transport)
        client = Client(protocol)
        transport.open()
        eoslib.set_client(client)
        
    def apply(self, _account, _action, code: bytes ):
        account = n2s(_account)
        action = n2s(_action)
        print(account, action, len(code))


        if account in self.modules:
            print('++++++++apply2')
            self.modules[account][0].apply(account, action)
            print('++++++++apply2 end')
        else:
            module_name = account
            new_module = imp.new_module(module_name)
            exec(code,vars(new_module))
            self.modules[account] = [new_module, code]
            print('+++apply')
            new_module.apply(_account, _action)
            print('+++apply end')

#        ret = client.db_get_i64(1)
#        print(ret)
        return 1122;

class TaskProcessor(TServer.TServer):
    """Simple single-threaded server that just pumps around one transport."""

    def __init__(self, *args):
        TServer.TServer.__init__(self, *args)

    def serve(self):
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
                logging.exception(x)
            print('clinet disconnected')
            itrans.close()
            otrans.close()

def start():
    handler = RequestHandler()
    processor = rpc_interface.Processor(handler)
    transport = TSocket.TServerSocket(HOST, APPLY_PORT)
    tfactory = TTransport.TBufferedTransportFactory()
    pfactory = TBinaryProtocol.TBinaryProtocolFactory()
    
    rpcServer = TaskProcessor(processor,transport, tfactory, pfactory)
    
    print('Listening for task:', HOST,':', APPLY_PORT)
    rpcServer.serve()

if __name__ == '__main__':
    start()

'''
def db_store_i64(self, scope: int, table: int, payer: int, id: int, buffer: bytes ):
    return 1

def db_update_i64(self, itr: int, payer: int, buffer: bytes ):
    return None

def db_remove_i64(self, itr: int ):
    return None

def db_get_i64(self, itr: int ):
    return b'hello,world'

def db_next_i64(self, itr: int):
    return (1, 12345)

def db_previous_i64(self, itr: int ):
    return (0, 123456)

def db_find_i64(self, code: int, scope: int, table: int, id: int ):
    return 1

def db_lowerbound_i64(self, code: int, scope: int, table: int, id: int ):
    return 2

def db_upperbound_i64(self, code: int, scope: int, table: int, id: int ):
    return 3

def db_end_i64(self, code: int, scope: int, table: int ):
    return 4

'''
