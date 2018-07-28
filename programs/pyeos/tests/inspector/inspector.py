import db
from eoslib import N, read_action

def assert_success(func):
    def func_wrapper(*args, **kwargs):
        try:
            func(*args, **kwargs)
        except Exception as e:
            print('++++test:', func, 'FAILED!')
            assert False
        print('++++test:', func, 'PASSED!')
        return True
    return func_wrapper

def assert_failed(func):
    def func_wrapper(*args, **kwargs):
        try:
            func(*args, **kwargs)
        except Exception as e:
            print('++++test:', func, 'PASSED!')
            return True
        print('++++test:', func, 'FAILED!')
        return False
    return func_wrapper


@assert_failed
def test_getattr():
    print('++++++++++++++++++++', 'test_getattr begin')
    a = db.__builtins__
    print(a)
    print('++++++++++++++++++++', 'test_getattr end')

@assert_failed
def test_setattr():
    db.get_i64 = 'abc'

@assert_failed
def test_code_object():
    type(test_code_object.__code__)(0, 0, 0, 0, 0, b'', (), (), (), '', '', 1, b'')

'''
code(argcount, kwonlyargcount, nlocals, stacksize, flags, codestring,
         constants, names, varnames, filename, name, firstlineno,
         lnotab[, freevars[, cellvars]])
'''

@assert_failed
def test_code_object2():
    (lambda fc=(
        lambda n: [
            c for c in 
                ().__class__.__bases__[0].__subclasses__() 
                if c.__name__ == n
            ][0]
        ):
        fc("function")(
            fc("code")(
            0,
            0,
            0,
            0,
            0,
            b"KABOOM",
            (None,),
            (),
            (),
            '',
            '',
            1,
            b'',
            (),
            ()
            ),{}
        )()
    )()

@assert_success
def test_builtin_types():
    a = {}
    a = dict()
    a[123] = 456
    print(a[123])
    a = [1, 2, 3]
    print(a[1])

@assert_failed
def test_builtins():
    __builtins__['pow']()

@assert_failed
def test_call_with_key_words():
    kwargs = {}
    exec(test_call_with_key_words.__code__, **kwargs)

def fake_func():
    pass

class A():
    def __init__(self):
        pass
    def sayHello(self):
        print('hello from A')

@assert_failed
def test_change_builtin_module():
    a = 'abc'
    a.fake_func = fake_func

@assert_success
def test_change_builtin_module2():
    a = A()
    a.fake_func = fake_func

@assert_success
def test_change_builtin_module3():
    a = A()
    a.sayHello()

@assert_failed
def test_import():
    import pickle


def test_crash1():
    raise SystemExit

def test_crash2():
    i=''
    for _ in range(9**6):i=filter(int,i)
    del i

def assert_failed(func):
    def func_wrapper(*args, **kwargs):
        try:
            func(*args, **kwargs)
        except Exception as e:
            print('++++test:', func, 'PASSED!')
            return True
        print('++++test:', func, 'FAILED!')
        return False
    return func_wrapper
def test_recursive_call():
    test_recursive_call()
    f = lambda f:f(f)
    f(f)

def apply(receiver, code, action):
#    test_crash1()
#    test_crash2()
    test_recursive_call()

    if 0:
        test_getattr()
        test_setattr()

        test_code_object()
        test_code_object2()

        test_builtin_types()
        test_builtins()

        test_call_with_key_words()

        test_change_builtin_module()
        test_change_builtin_module3()
        test_change_builtin_module2()
        test_import()
