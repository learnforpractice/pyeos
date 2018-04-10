#include <Python.h>
#include <stdio.h>
#include <boost/thread/thread.hpp>

#include <eosio/chain/apply_context.hpp>
#include <fc/exception/exception.hpp>
#include <appbase/application.hpp>

#include "rpc_interface.hpp"

extern "C" void PyInit_eoslib_();
extern "C" void PyInit_rpc_interface_();
extern "C" void PyInit_mytest();


void eos_main();
typedef void (*fn_init)();
static fn_init s_init_eos = 0;

void init_eos() {
   if (s_init_eos) {
      s_init_eos();
   }
}
extern "C" int init_mypy(fn_init _init, bool rpc_enabled) {
   Py_InitializeEx(0);
//   _Py_InitializeEx_Private(0, 1);
   s_init_eos = _init;

#ifdef WITH_THREAD
   PyEval_InitThreads();
#endif

   PyInit_eoslib_();
   PyInit_rpc_interface_();
   PyRun_SimpleString("import eoslib_");

   PyRun_SimpleString(
       "import sys;"
        "sys.path.append('../../libraries/chain/rpc_interface');"
   );
   if (rpc_enabled) {
      wlog("run eosserver");
      PyRun_SimpleString("import eosserver;eosserver.start()");
   } else {
      wlog("init_eos");
      init_eos();
   }

   while(true) {
      boost::this_thread::sleep_for(boost::chrono::milliseconds(1000000));
   }

   //   PyRun_SimpleString("initrpc.init()");

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
