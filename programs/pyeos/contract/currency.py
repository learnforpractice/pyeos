if __name__ == '__main__':
	import eoslib_dummy as eoslib
else:
	import eoslib
import struct

code = eoslib.N(b'currency')
table = eoslib.N(b'account')

class Account(object):
	key = eoslib.N(b'account')
	def __init__(self,scope,balance=0):
		self.scope = scope
		if balance == 0:
			self.load()
		else:
			self.balance = balance
	def isEmpty(self):
		return balance == 0
	def store(self):
		eoslib.store_u64(self.scope,code,table,Account.key,self.balance)
	def load(self):
		self.balance = eoslib.load_u64(self.scope,code,table,Account.key)

def init():
#	print('hello from init')
	a = Account(code,100000)
	a.store()

def apply(name,type):
#	print('hello from python apply',name,type)
#	print(eoslib.n2s(name),eoslib.n2s(type))
	if type == eoslib.N(b'transfer'):
		msg = eoslib.readMessage()
		result = struct.unpack('QQQ',msg)
#		print(result)
		from_ = result[0]
		to_ = result[1]
		amount = result[2]
		
		eoslib.requireAuth( from_ );
		eoslib.requireNotice( from_ );
		eoslib.requireNotice( to_ )

		from_ = Account(from_)
		to_ = Account(to_)
		if from_.balance >= amount:
			from_.balance -= amount
			to_.balance += amount
			from_.store()
			to_.store()

if __name__ == '__main__':
	init()
	apply(eoslib.N(b'python'),eoslib.N(b'transfer'))



