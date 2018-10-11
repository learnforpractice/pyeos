#include "pytest.hpp"
#include <appbase/application.hpp>
#include <eosiolib_native/vm_api.h>

using namespace eosio::testing;

extern "C" void vm_manager_init();
//eosapi.pyx
void py_exit();

extern "C" {
   void init_api();
   PyObject* PyInit_eosapi();
   PyObject* PyInit_wallet();
   PyObject* PyInit_net();

//only used in debugging
   PyObject* PyInit_eoslib();
   PyObject* PyInit_db();
   PyObject* PyInit_rodb();
   PyObject* PyInit_debug();
   PyObject* PyInit_python_contract();
   PyObject* PyInit__struct();
   PyObject* PyInit_pyobject();

   //vm_manager.cpp
   void vm_deinit_all();
   void vm_api_init();
   void vm_manager_init();
}

void init_console() {
   Py_InitializeEx(0);
    PyRun_SimpleString("print('hello,world')");

    PyEval_InitThreads();

   PyInit_pyobject();
   PyInit_wallet();
   PyInit_eosapi();
   PyInit_net();

   PyInit_rodb();
   PyInit_debug();
//   PyInit_python_contract();

   PyRun_SimpleString(
       "import sys;"
        "sys.path.append('../../programs/pyeos');"
        "sys.path.append('../../programs/pyeos/contracts');"
   );
    PyRun_SimpleString("print('hello,world')");

    PyRun_SimpleString("import readline");
   PyRun_SimpleString("import wallet");
   PyRun_SimpleString("import eosapi;");
   PyRun_SimpleString("import debug;");
   PyRun_SimpleString("from imp import reload;");
   PyRun_SimpleString("import initeos;initeos.preinit()");
   PyRun_SimpleString("import initeos;initeos.init_wallet()");
}

bool is_unittest_mode() {
   return true;
}

void init_wallet();
int main(int argc, char** argv) {
   init_wallet();
   get_vm_api()->is_unittest_mode = is_unittest_mode;

   appbase::app().initialize<>(argc, argv);
   init_console();
   tester main;
   try {
      main.create_account(N(newacc));
//      auto b = main.produce_block();
   } FC_LOG_AND_DROP();

   PyRun_SimpleString("print('hello,world')");
   PyRun_SimpleString("import initeos");
   PyRun_SimpleString("initeos.start_console()");

   appbase::app().quit();
   return 1;
}

