def auth(func):
    def func_wrapper(*args):
        print('TODO: authorization check')
        return func(*args)
    return func_wrapper

@auth
def add(a, b):
    return a+b
