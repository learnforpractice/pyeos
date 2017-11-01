import eosapi
import wallet
def init():
    psw = 'PW5KTHfg4QA7wD1dZjbkpA97hEktDtQaip6hNNswWkmYo5pDK3CL1'
    wallet.open('mywallet')
    wallet.unlock('mywallet',psw)
import time
def t():
    for i in range(1,1000):
#        print(i)
        if i%200 == 0:
            time.sleep(5.0)
            print(i)
        r = eosapi.push_message('currency','transfer','{"from":"currency","to":"inita","amount":%d}'%(i,),['currency','inita'],{'currency':'active'})

def test_db():
    import eostest
    eostest.start()
    eostest.create_account_manually(b'hello')
    eostest.get_account(b'hello')
    eostest.end()
    
    
    