#pragma once
#include <Python.h>

#include <string>
#include <vector>

using namespace std;

string connect_(const string& host);
string disconnect_(const string& host);
PyObject* status_(const string& host);

PyObject* connections_();
