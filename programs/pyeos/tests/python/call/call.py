import struct
import eoslib
from eoslib import N

def apply(receiver, code, action):

    if action == N('sayhello'):
        eoslib.require_auth(N('call'))
        msg = eoslib.read_action()
        print(msg.decode('utf8'))
    elif action == N('call'):
        msg = eoslib.read_action()
        _from, to, amount = struct.unpack('QQQ', msg)
        symbol=bytearray(8)
        symbol[0] = 4
        symbol[1] = ord('E')
        symbol[2] = ord('O')
        symbol[3] = ord('S')
        memo = eoslib.pack_bytes(b'hello')
        args = struct.pack('QQQ8s%ds'%(len(memo),), _from, to, amount, symbol, memo)

        print('++++call')
        eoslib.send_inline(N('eosio.token'), N('transfer'), args, {'call':'active'})

#        eoslib.call_set_args(args)
#        eoslib.call(eoslib.s2n('eosio.token'), eoslib.s2n('transfer'))
