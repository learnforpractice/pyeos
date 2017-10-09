if not __name__ == '__main__':
    import eosapi
    import wallet
    psw = 'PW5KTHfg4QA7wD1dZjbkpA97hEktDtQaip6hNNswWkmYo5pDK3CL1'
    wallet.open('mywallet')
    wallet.unlock('mywallet',psw)

def test():
#from contract.exchange import test
#test.test()
    r = eosapi.push_message('eos','transfer','{"from":"inita","to":"exchange","amount":10,"memo":"hello"}',['exchange','inita'],{'inita':'active'})
    r = eosapi.push_message('eos','transfer','{"from":"exchange","to":"inita","amount":1,"memo":"hello"}',['exchange','inita'],{'exchange':'active','inita':'active'})

    args = {"from":"inita","to":"exchange","amount":10}
    scopes = ['exchange','inita']
    permissions = {'inita':'active'}
    r = eosapi.push_message('currency','transfer',args,scopes,permissions)

'''
         "buyer" : "OrderID",
         "price" : "UInt128",
         "quantity" : "UInt64",
         "expiration" : "Time"
    fill_or_kill
'''
if __name__ == '__main__':
    import sys
    sys.path.insert(0,'..')
    from exchange import *
    import eoslib as eoslib
    from eoslib import N,readMessage,writeMessage
    
    init()
    writeMessage(bytes(512))
    order = BuyOrder()
    order.buyer = OrderID()
    order.buyer.name = eoslib.N(b'buyer')
    order.buyer.id = 0
    
    order.price = uint128(11)
    order.quantity = 12
    order.expiration = int(time.time())+100
    writeMessage(order())
    apply(exchange,N(b'buy'))
    
    order = SellOrder()
    order.seller = OrderID()
    order.seller.name = eoslib.N(b'seller')
    order.seller.id = 0
    
    order.price = uint128(11)
    order.quantity = 12
    order.expiration = int(time.time())+100
    writeMessage(order())
    
    apply(exchange,N(b'sell'))

    id = OrderID()
    id.name = eoslib.N(b'buyer')
    id.id = 0
    writeMessage(id())

    apply(exchange,N(b'cancelbuy'))
    apply(exchange,N(b'cancelsell'))

    msg = struct.pack('QQQ',123,exchange,1)
    msg += b'hello'
    writeMessage(msg)
    apply(currency,N(b'transfer'))

    msg = struct.pack('QQQ',123,exchange,1)
    msg += b'hello'
    writeMessage(msg)
    apply(N(b'eos'),N(b'transfer'))
    print('done!')