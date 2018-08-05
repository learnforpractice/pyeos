import struct
import eoslib

def apply(receiver, code, action):
    
    _from = eoslib.s2n('eosio')
    to = eoslib.s2n('hello')
    amount = 10*10000
    symbol=bytearray(8)
    symbol[0] = 4
    symbol[1] = ord('E')
    symbol[2] = ord('O')
    symbol[3] = ord('S')
    memo = b'hello'
    args = struct.pack('QQQ8s{}s'.format(len(memo)), _from, to, amount, symbol, memo)
    eoslib.call_set_args(args)
    eoslib.call(eoslib.s2n('eosio.token'), eoslib.s2n('transfer'))

    if action == N('sayhello'):
        require_auth(N('call'))
        msg = read_action()
        print(msg.decode('utf8'))
