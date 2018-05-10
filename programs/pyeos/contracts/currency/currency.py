from eoslib import read_action, N, n2s, unpack
from backyard.token import create, issue, transfer

try:
    import ustruct as struct 
except Exception as e:
    import struct

code = N(b'currency')
table = N(b'account')

#{"to":"currency","quantity":"1000.0000 CUR","memo":""}
def issue_():
    iss = issue()
#    iss.p()

#{"from":"currency","to":"eosio","quantity":"20.0000 CUR","memo":"my first transfer"}
def transfer_():
    t = transfer()
#    t.p()

def create_():
    c = create()
#    c.p()

def apply(receiver, code, action):
#    print('++++++apply:', n2s(name))
    if action == N('transfer'):
        transfer_()
    elif action == N('issue'):
        issue_()
    elif action == N('create'):
        create_()

