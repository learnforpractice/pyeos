import eoslib
try:
    import struct
except Exception as e:
    import ustruct as struct
mp=eoslib.N('micropython')
print('hello,world from mp')
def init():
    print('hello from micropython.init')
    print('--------------------------------')
    db_keys = struct.pack('Q', eoslib.N('currency'))
    values = struct.pack('QQ', 0x778899, 0x112233)
    print("mp:", mp)
    print(eoslib.uint64_to_string(mp))
    ret = eoslib.store(mp, mp, db_keys, 0, values)
    print('++++++++eoslib.store return:',ret)

def apply(name, type):
    print(name, type)

