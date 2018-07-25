#include <string>
#include <vector>
#include <Python.h>

#include "inspector.hpp"

using namespace std;

void get_code(uint64_t account, string& code);
void get_c_string(PyObject* s, string& out);

//bltinmodule.c
extern "C" PyObject* builtin_exec_(PyObject *source, PyObject *globals, PyObject *locals);
