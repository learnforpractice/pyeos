# -*- coding: utf8 -*-
from struct import pack, unpack

from threading import Thread, Event
from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol

from base import  MyBinaryProtocol, MyBinaryProtocolFactory

#server side
from thrift.server import TServer
from idl import eoslib_service
from idl import ttypes

#client side
from idl.rpc_interface import Client
from idl.rpc_interface import Result

import time
try:
    import eoslib_
except:
    pass
import rpc_interface_

HOST = 'localhost'
APPLY_PORT = 9091
DB_PORT = 9092

rpc_enabled = False

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
    def read_action(self):
        return eoslib_.read_action()

    def db_store_i64(self, scope: int, table: int, payer: int, id: int, buffer: bytes ):
        return eoslib_.store_i64(scope, table, payer, id, buffer)

    def db_update_i64(self, itr: int, payer: int, buffer: bytes ):
        eoslib_.update_i64(itr, payer, buffer)

    def db_remove_i64(self, itr: int ):
        eoslib_.remove_i64(itr) 

    def db_get_i64(self, itr: int ):
        return eoslib_.get_i64(itr)

    def db_next_i64(self, itr: int):
        itr_next, primary = eoslib_.next_i64(itr)
        return Result(itr_next, primary)

    def db_previous_i64(self, itr: int ):
        itr_next, primary = eoslib_.previous_i64(itr)
        return Result(itr_next, primary)

    def db_find_i64(self, code: int, scope: int, table: int, id: int ):
        return eoslib_.find_i64(code, scope, table, id)
    
    def db_lowerbound_i64(self, code: int, scope: int, table: int, id: int ):
        return eoslib_.lowerbound_i64(code, scope, table, id)
    
    def db_upperbound_i64(self, code: int, scope: int, table: int, id: int ):
        return eoslib_.upperbound_i64(code, scope, table, id)

    def db_end_i64(self, code: int, scope: int, table: int ):
        return eoslib_.end_i64(code, scope, table)

client = None
def open_client():
    global client
    while True:
        try:
            tsocket = TSocket.TSocket(HOST, APPLY_PORT)
            transport = TTransport.TBufferedTransport(tsocket)
            protocol = MyBinaryProtocol(transport)
            client = Client(protocol)
            transport.open()
            print('open client return')
            break
        except Exception as e:
            print('+++++++exception occurred:',e)
            time.sleep(3.0)

def send_apply():
    client.apply(0, 0, None)

class DBServer(TServer.TServer, Thread):

    def __init__(self, *args):
        Thread.__init__(self, daemon=True)
        TServer.TServer.__init__(self, *args)
        self._shutdown = False
        self.shutdownevent = Event()

        self.itrans = None
        self.otrans = None

    def run(self):
        self.serve()

    def serve(self):
        print('//Starting the rpc server at', HOST,':', DB_PORT, ', waiting for connection')
        self.serverTransport.listen()
        while True:
            client = self.serverTransport.accept()
            print('accpet return: ', client)
            if self._shutdown:
                self.shutdownevent.set()
                return
            if not client:
                continue
            print('client connected', client)
            self.client = client

            rpc_interface_.start_eos()

            itrans = self.inputTransportFactory.getTransport(client)
            otrans = self.outputTransportFactory.getTransport(client)
            iprot = self.inputProtocolFactory.getProtocol(itrans)
            oprot = self.outputProtocolFactory.getProtocol(otrans)
            self.itrans = itrans
            self.otrans = otrans
            
            try:
                while True:
                    self.processor.process(iprot, oprot)
            except TTransport.TTransportException:
                pass
            except Exception as x:
                logger.exception(x)

            print('clinet disconnected')

            if self._shutdown:
                self.shutdownevent.set()
                return

            itrans.close()
            otrans.close()
        print('server out!')

    def shutdown(self):
        self._shutdown = True
        self.client.close()

        if self.itrans:
            self.itrans.close()
            self.otrans.close()
        print("wait for shutdown")
        self.shutdownevent.wait(2.0)
        print("wait for shutdown return")

rpcServer = None
def start():
    global rpcServer
    open_client()
    print('start server')
    handler = RequestHandler()
    
    processor = eoslib_service.Processor(handler)
    transport = TSocket.TServerSocket(HOST, DB_PORT)
    tfactory = TTransport.TBufferedTransportFactory()
    pfactory = MyBinaryProtocolFactory()

    rpcServer = DBServer(processor,transport, tfactory, pfactory)
#    rpcServer.start()
    rpcServer.serve()

def shutdown():
    global rpcServer
    if rpcServer:
        print('rpc server is shutting down');
#        rpcServer.shutdown()
        rpcServer.stop()


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

#    print('done!', ret)
    return ret

if __name__ == '__main__':
    start()
    rpcServer.join()

