import eosapi
def dlbw(_from, _to, net, cpu):
    args = {'from':_from, 
            'receiver':_to, 
            'stake_net_quantity':'%.4f EOS'%(net,), 
            'stake_cpu_quantity':'%.4f EOS'%(cpu,), 
            'transfer':False
            }
    eosapi.push_action('eosio', 'delegatebw', args, {_from:'active'})

def t1():
    while True:
        import time
        import traceback
        try:
            ct1.test3(500)
            time.sleep(1.0)
        except Exception as e:
            traceback.print_exc()
            time.sleep(1.0)


def t2():
    while True:
        import time
        try:
            ht.test3()
            time.sleep(1.0)
        except Exception as e:
            print(e)
            time.sleep(1.0)

def t3():
    while True:
        import time
        try:
            it.test3()
            time.sleep(1.0)
        except Exception as e:
            print(e)
            time.sleep(1.0)
