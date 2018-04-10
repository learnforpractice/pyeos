from multiprocessing import Process
import os
import eosserver


def f(name):
    import eosclient
    eosclient.start

p = None
def init():
    global p
    print('+++++++++++initrpc++++++++++++++')
    eosserver.start()
    p = Process(target=f, args=('',))
    p.start()

if __name__ == '__main__':
    init()



