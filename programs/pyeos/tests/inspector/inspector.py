import db
from eoslib import N, read_action

def assert_success(func):
    def func_wrapper(*args, **kwargs):
        try:
            func(*args, **kwargs)
        except Exception as e:
            print('++++test:', func, 'FAILED!')
            return False
        except:
            print('++++test:', func, 'FAILED!', 'unknown exception!')
            return False
        print('++++test:', func, 'PASSED! ')
        return True
    return func_wrapper

def assert_failure(func):
    def func_wrapper(*args, **kwargs):
        try:
            func(*args, **kwargs)
        except Exception as e:
            print('exception:', e)
            print('++++test:', func, 'PASSED!')
            return True
        except:
            print('++++test:', func, 'PASSED!', 'unknow exception!')
            return True
        print('++++test:', func, 'FAILED!')
        return False
    return func_wrapper


@assert_failure
def test_getattr():
    print('++++++++++++++++++++', 'test_getattr begin')
    a = db.__builtins__
    print(a)
    print('++++++++++++++++++++', 'test_getattr end')

@assert_failure
def test_setattr():
    db.get_i64 = 'abc'

@assert_failure
def test_code_object():
    type(test_code_object.__code__)(0, 0, 0, 0, 0, b'', (), (), (), '', '', 1, b'')

'''
code(argcount, kwonlyargcount, nlocals, stacksize, flags, codestring,
         constants, names, varnames, filename, name, firstlineno,
         lnotab[, freevars[, cellvars]])
'''

@assert_failure
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

@assert_failure
def test_builtins():
    __builtins__['pow']()

@assert_failure
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

@assert_failure
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

@assert_failure
def test_change_builtin_module4():
    a = A()
    print(a.__class__)

@assert_failure
def test_import():
    import pickle

@assert_failure
def test_import2():
    __import__("db")

@assert_failure
def test_crash1():
    raise SystemExit

@assert_failure
def test_crash2():
    i=''
    for _ in range(9**6):i=filter(int,i)
    del i

@assert_failure
def test_recursive_call():
    f = lambda f:f(f)
    f(f)

@assert_failure
def test_delete1():
    del db.__builtins__

@assert_failure
def test_delete2():
#call PyObject_SetAttr(owner, name, (PyObject *)NULL);
    del db.find_i64

@assert_success
def test_delete3():
#call PyObject_SetAttr(owner, name, (PyObject *)NULL);
    a = {1:2}
    del a[1]

@assert_failure
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

@assert_failure
def test_context_manager():
    with EvilCM():
        raise Exception()

@assert_failure
def test_memory_out():
    a = bytes(600*1024)

@assert_success
def test_memory_out2():
    for i in range(10):
        a = bytes(30*1024)
        del a


@assert_success
def test_base_exception():
    try:
        print('raise BaseException')
        raise BaseException('')
    except Exception:
        print('oops')

@assert_success
def test_base_exception2():
    try:
        raise BaseException('')
    except:
        print('oopss')

@assert_failure
def test_create_module():
    module = type(db)
    for i in range(10):
        mod = module("abc")

@assert_failure
def test_set_func():
    a = db.get_i64
    a.a = 123

def apply(receiver, code, action):
#    Exception()
#    KeyboardInterrupt()
#    return
#    test_create_module()
    test_set_func()
    return

    if 1:
        test_crash2()
    #    test_base_exception()
        test_base_exception2()
        test_memory_out()
        test_memory_out2()

    if 1:
        test_crash1()
    #    test_crash2()
        test_recursive_call()
    
        test_delete1()
        test_delete2()
        test_delete3()
    if 1:
        test_reload()
        test_context_manager()
        test_import2()

#----------------------------
    
    test_getattr()
    test_setattr()

#    test_code_object()
    test_code_object2()

    test_builtin_types()
    test_builtins()
#--------------------------

    test_call_with_key_words()

    test_change_builtin_module()
    test_change_builtin_module2()
    test_change_builtin_module3()
    test_change_builtin_module4()

    test_import()
