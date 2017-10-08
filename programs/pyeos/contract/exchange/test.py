

def test():
    pass

if __name__ == '__main__':
    from exchange import *
    import eoslib_dummy as eoslib
    from eoslib_dummy import N,readMessage,writeMessage
    
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