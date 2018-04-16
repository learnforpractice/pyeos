#include <boost/asio.hpp>
#include <fc/exception/exception.hpp>
#include <fc/io/json.hpp>
#include <fc/variant.hpp>
#include <iostream>
#include <string>
#include <vector>

#include <eosio/account_history_api_plugin/account_history_api_plugin.hpp>
#include <eosio/chain/config.hpp>
#include <eosio/chain_plugin/chain_plugin.hpp>

#include <boost/algorithm/string/split.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm/sort.hpp>
#include <eosio/utilities/key_conversion.hpp>

#include <IR/Module.h>
#include <IR/Validate.h>
#include <Inline/BasicTypes.h>
#include <Runtime/Runtime.h>
#include <WASM/WASM.h>
#include <WAST/WAST.h>

#include <fc/io/fstream.hpp>

#include <Python.h>
#include <boost/thread.hpp>
#include <eos/py_plugin/py_plugin.hpp>
#include <fc/log/logger_config.hpp>

#include <eosio/chain/exceptions.hpp>
#include <eosio/chain_api_plugin/chain_api_plugin.hpp>
#include <fc/io/json.hpp>

using namespace std;
using namespace eosio;
using namespace eosio::chain;
using namespace eosio::utilities;

namespace eosio {

class py_plugin_impl {
  public:
};

py_plugin::py_plugin() : my(new py_plugin_impl()) {}
py_plugin::~py_plugin() {}

void py_plugin::set_program_options(options_description&,
                                    options_description& cfg) {
/*
   cfg.add_options()("interactive,i", bpo::bool_switch()->default_value(false),
                     "enter interactive mode")("option-name", bpo::value<string>()->default_value("default value"), "Option Description");
*/
}

void py_plugin::plugin_initialize(const variables_map& options) {
/*
   if (options.count("interactive")) {
      if (options.at("interactive").as<bool>()) {
         interactive = true;
      }
   }
   */
}

extern "C" void c_printf(const char* s);
extern "C" void PyInit_eosapi();
extern "C" PyObject* PyInit_eostypes_();
extern "C" PyObject* PyInit_wallet();
extern "C" PyObject* PyInit_hello();
extern "C" PyObject* PyInit_python_contract();

void py_thread() {
//   PyImport_AppendInittab("hello", PyInit_hello);

   Py_Initialize();
#if 0
    PyRun_SimpleString("import readline");
    PyInit_eosapi();
    PyInit_eostypes_();
    PyInit_wallet();
    PyInit_python_contract();
    PyRun_SimpleString("import wallet;");
    PyRun_SimpleString("import eosapi;import sys;sys.path.append('./eosd')");
    PyRun_SimpleString("from initeos import *");
#endif

   ilog("++++++++++++++py_plugin::plugin_startup");
   //    get_info();
   PyRun_InteractiveLoop(stdin, "<stdin>");
   Py_Finalize();
}

void py_plugin::plugin_startup() {
   //    boost::thread t{py_thread};
   ilog("py_plugin::plugin_startup");
   //   chain_controller& db = app().get_plugin<chain_plugin>().chain();
}

void py_plugin::plugin_shutdown() {
   ilog("py_plugin::plugin_shutdown()");
   //    Py_Finalize();
}

}  // namespace eos
