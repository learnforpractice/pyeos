#include <Python.h>
#include <stdio.h>

#include <eosio/chain/apply_context.hpp>
#include <fc/exception/exception.hpp>

#include "rpc_interface.hpp"

extern "C" void PyInit_utility();
extern "C" void PyInit_eoslib();
extern "C" void PyInit_rpc_interface();

extern "C" int init_mypy() {
   Py_InitializeEx(0);
//   _Py_InitializeEx_Private(0, 1);

#ifdef WITH_THREAD
   PyEval_InitThreads();
#endif

   PyInit_utility();
   PyInit_eoslib();
   PyRun_SimpleString(
       "import sys;"
        "sys.path.append('../../libraries/chain/rpc_interface');"
         "import eosserver;"
   );
   PyInit_rpc_interface();
   PyRun_SimpleString("import initrpc");
   PyRun_SimpleString("initrpc.init()");

//   Py_Finalize();
   return 0;
}

int rpc_interface_apply(uint64_t account, uint64_t action, string& code);

namespace eosio {
namespace chain {


rpc_interface::rpc_interface() {
}

rpc_interface& rpc_interface::get() {
   static rpc_interface* python = nullptr;
   if (!python) {
      wlog("rpc_interface::init");
      python = new rpc_interface();
   }
   return *python;
}

void rpc_interface::on_setcode(uint64_t _account, bytes& code) {
}

void rpc_interface::apply(apply_context& c, const shared_vector<char>& code) {
   string _code = string(code.data(), code.size());

   try {
      rpc_interface_apply(c.act.account.value, c.act.name.value, _code);
   } catch (fc::exception& ex) {
      elog(ex.to_detail_string());
   } catch (boost::exception& ex) {
      elog(boost::diagnostic_information(ex));
   } catch(...) {
      wlog("unkown exception!");
   }
}


}
}
