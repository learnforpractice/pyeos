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
      PyObject* database_create(string& path, bool readonly=true);
      PyObject* database_create_account(void* db, string& name);
      PyObject* database_get_account(void* db, string& name);
      PyObject* database_get_recent_transaction(void* db, string& id);
      PyObject* database_get_code(void* db, string& code, int& type);
      PyObject* database_flush(void* db, string& id);

   }
}

#endif /* DATABASE__HPP_ */
