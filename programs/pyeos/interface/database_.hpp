/*
 * database_.hpp
 *
 *  Created on: Sep 11, 2017
 *      Author: newworld
 */

#ifndef DATABASE__HPP_
#define DATABASE__HPP_
#include <string>
#include <Python.h>

using namespace std;

namespace python {
   namespace database {
      PyObject* database_create(string& path);
   }
}

#endif /* DATABASE__HPP_ */
