# -*- coding: utf8 -*-
import imp
import logging
from threading import Thread, Timer, Event
from struct import pack, unpack
from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol

from base import  MyBinaryProtocol, MyBinaryProtocolFactory, IPCSocket, IPCServer
from pipe import PipeServer, PipeClient

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
import ipc
ipc.remove(b'server1')
ipc.remove(b'server2')
ipc.remove(b'client1')
ipc.remove(b'client2')

HOST = 'localhost'
APPLY_PORT = 9091
DB_PORT = 9092

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

class RequestHandler(object):
    def __init__(self):
        self.modules = {}

    def apply(self, _account, _action ):
        account = n2s(_account)
        action = n2s(_action)

        if account in self.modules:
            self.modules[account][0].apply(_account, _action)
        else:
            _code = eoslib.get_code(_account)
            module_name = account
            new_module = imp.new_module(module_name)
            exec(_code,vars(new_module))
            self.modules[account] = [new_module, _code]
            new_module.apply(_account, _action)

#        ret = client.db_get_i64(1)
#        print(ret)
        return 1122;

class TaskProcessor(TServer.TServer, Thread):
    """Simple single-threaded server that just pumps around one transport."""

    def __init__(self, *args):
        TServer.TServer.__init__(self, *args)
        Thread.__init__(self)

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

    def run(self):
        self.serve()

from code import InteractiveConsole
from imp import new_module
 
class Console(InteractiveConsole):
 
    def __init__(self, names=None):
        names = names or {}
        names['console'] = self
        InteractiveConsole.__init__(self, names)
        self.superspace = new_module('superspace')
        self.runsource('from base import  MyBinaryProtocol, MyBinaryProtocolFactory, IPCSocket, IPCServer')
        self.runsource('import ipc')

    def enter(self, source):
        source = self.preprocess(source)
        self.runcode(source)
 
    @staticmethod
    def preprocess(source):
        return source
 
def start_console():
    print("++++++++++start_console++++++++++++++")
    console = Console()
    console.interact()

def start():
    import sys;
    sys.path.append('/Users/newworld/dev/pyeos/libraries/python/pysrc')
    import pydevd;pydevd.settrace(suspend=False)
    
#    start_console()
#    return

    t = Thread(target = start_console)
    t.start()

    transport = PipeClient('1')
    protocol = MyBinaryProtocol(transport)
    client = Client(protocol)
    transport.open()
    eoslib.set_client(client)
    transport.write(b'hello')
    print('write return...')

    handler = RequestHandler()
    processor = rpc_interface.Processor(handler)
#    transport = TSocket.TServerSocket(HOST, APPLY_PORT)
    transport = PipeServer('2')
    tfactory = TTransport.TBufferedTransportFactory()
    pfactory = MyBinaryProtocolFactory()

    rpcServer = TaskProcessor(processor,transport, tfactory, pfactory)

    print('Listening for task:', HOST,':', APPLY_PORT)
    rpcServer.serve()


if __name__ == '__main__':
    import pydevd;pydevd.settrace()
    start()
