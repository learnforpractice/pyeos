import eoslib
import struct
def init():
	print('hello from init')
def apply(name,type):
	print('hello from python apply',name,type)
	msg = eoslib.readMessage()
	result = struct.unpack('QQQ',msg)
	print(result)
	from_ = result[0]
	to_ = result[1]
	amount = result[2]
	eoslib.requireNotice( to_ )
	eoslib.requireNotice( from_ );
	eoslib.requireAuth( from_ );

