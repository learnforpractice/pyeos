import db
from eoslib import N, read_action

def assert_success(func):
    def func_wrapper(*args, **kwargs):
        try:
            func(*args, **kwargs)
        except Exception as e:
            print('++++test:', func, 'FAILED!')
            assert False
        print('++++test:', func, 'PASSED! ')
        return True
    return func_wrapper

def assert_failed(func):
    def func_wrapper(*args, **kwargs):
        try:
            func(*args, **kwargs)
        except Exception as e:
            print('exception:', e)
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

@assert_failed
def test_import2():
    __import__("db")


def test_crash1():
    raise SystemExit

def test_crash2():
    i=''
    for _ in range(9**6):i=filter(int,i)
    del i

@assert_failed
def test_recursive_call():
    f = lambda f:f(f)
    f(f)

@assert_failed
def test_delete1():
    del db.__builtins__

@assert_failed
def test_delete2():
#call PyObject_SetAttr(owner, name, (PyObject *)NULL);
    del db.find_i64

@assert_success
def test_delete3():
#call PyObject_SetAttr(owner, name, (PyObject *)NULL);
    a = {1:2}
    del a[1]

@assert_failed
def test_reload():
    import imp
    imp.reload(db)

class EvilCM(object):
    def __enter__(self):
        return self
    def __exit__(self, exc_type, exc, tb):
        print(dir(tb))
        print('++++++++++++++++++:', exc_type, exc, tb)
        print(tb.tb_next.tb_frame.f_locals)
#        tb.tb_next.tb_frame.f_locals['open_file']('w00t', 'w').write('yay!\n')
        return True

@assert_failed
def test_context_manager():
    with EvilCM():
        raise Exception()

@assert_failed
def test_memory_out():
    a = bytes(100*1024)

@assert_success
def test_memory_out2():
    for i in range(10):
        a = bytes(30*1024)
        del a


@assert_success
def test_base_exception():
    try:
        raise BaseException('')
    except Exception:
        print('oops')

@assert_success
def test_base_exception2():
    try:
        raise BaseException('')
    except:
        print('oops')

def apply(receiver, code, action):
    test_base_exception()
#    test_memory_out()
#    test_memory_out2()

#    test_crash1()
#    test_crash2()
#    test_recursive_call()
#    test_delete()
#    test_delete3()
#    test_reload()
#    test_context_manager()
#    test_import2()
    print(int.to_bytes)

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
