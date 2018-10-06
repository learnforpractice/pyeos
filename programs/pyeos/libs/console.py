import os
import sys
import imp
import traceback
from code import InteractiveConsole

class PyEosConsole(InteractiveConsole):
    def __init__(self, locals):
        super(PyEosConsole, self).__init__(locals=locals, filename="<console>")

    def check_module(self):
        for k in sys.modules:
            module = sys.modules[k]
            if not hasattr(module, '__file__'):
                continue

            if not module.__file__:
                continue

            if not module.__file__.endswith('.py'):
                continue

            try:
                t1 = os.path.getmtime(module.__file__)
                t2 = os.path.getmtime(module.__cached__)
            except Exception as e:
                continue

            try:
                if t1 > t2:
                    print('Reloading ', module.__file__)
                    imp.reload(module)
            except Exception as e:
                traceback.print_exc()
                return False
        return True

    def interact(self, banner=None, exitmsg=None):
        try:
            sys.ps1
        except AttributeError:
            sys.ps1 = ">>> "
        try:
            sys.ps2
        except AttributeError:
            sys.ps2 = "... "
        cprt = 'Type "help", "copyright", "credits" or "license" for more information.'
        if banner is None:
            self.write("Python %s on %s\n%s\n(%s)\n" %
                       (sys.version, sys.platform, cprt,
                        self.__class__.__name__))
        elif banner:
            self.write("%s\n" % str(banner))
        more = 0
        while 1:
            try:
                if more:
                    prompt = sys.ps2
                else:
                    prompt = sys.ps1

                try:
                    line = self.raw_input(prompt)
                except EOFError:
                    self.write("\n")
                    break
                else:
                    if line.strip() == 'exit()':
                        break
                    if line.strip():
                        self.check_module()
                    more = self.push(line)
            except KeyboardInterrupt:
                self.write("\nKeyboardInterrupt\n")
                self.resetbuffer()
                more = 0
                break
            except:
                traceback.print_exc()
        if exitmsg is None:
            self.write('now exiting %s...\n' % self.__class__.__name__)
        elif exitmsg != '':
            self.write('%s\n' % exitmsg)
