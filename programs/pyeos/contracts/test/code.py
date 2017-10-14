import eoslib
import struct
from eoslib import N
def init():
	print('hello from init')

test = N(b'test')

def apply(code,action):
	eoslib.requireAuth(test)
	if code == test:
		if action == N(b'transfer'):
			msg = eoslib.readMessage()
			result = struct.unpack('QQQ',msg)
			print(result)
			from_ = result[0]
			to_ = result[1]
			amount = result[2]
		elif action == N(b'test'):
			msg = eoslib.readMessage()
			print(len(msg))
			result = int.from_bytes(msg[:8],'little')
			size = msg[8]
			for i in range(size):
				result = int.from_bytes(msg[9+i*8:9+i*8+8],'little')
				print(result)
			keys = msg[:8]
			values = msg[8:]
			eoslib.store(test,test,test,keys,0,values)
			keys = msg[:8]
			values = msg[8:]
			eoslib.load(test,test,test,keys,0,0,values)
			print(values)
		elif action == N(b'game'):
			msg = eoslib.readMessage()
			print(len(msg))
			result = int.from_bytes(msg[:32],'little')
			size = msg[32]
			for i in range(size):
				result = int.from_bytes(msg[9+i*8:9+i*8+8],'little')
				print(result)
			keys = msg[:8]
			values = msg[8:]
			eoslib.store(test,test,N('game'),keys,0,values)
			keys = msg[:8]
			values = msg[8:]
			eoslib.load(test,test,N('game'),keys,0,0,values)
			print(values)
