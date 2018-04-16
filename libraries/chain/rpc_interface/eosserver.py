# -*- coding: utf8 -*-
import socketserver_ as server
#from pipeserver import apply, start
#from ipcserver import apply, start


def start():
    server.start()

def apply(account, action):
    return server.apply(account, action)
