from eoslib import require_auth, N

def auth(func):
    def func_wrapper(*args):
        print('TODO: authorization check')
        return func(*args)
    return func_wrapper

@auth
def play():
    print('playing in garden')
