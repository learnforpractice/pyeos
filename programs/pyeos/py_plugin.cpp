#include <string>
#include <vector>
#include <boost/asio.hpp>
#include <iostream>
#include <fc/variant.hpp>
#include <fc/io/json.hpp>
#include <fc/exception/exception.hpp>

#include <eos/chain/config.hpp>
#include <eos/chain_plugin/chain_plugin.hpp>
#include <eos/account_history_api_plugin/account_history_api_plugin.hpp>

#include <eos/utilities/key_conversion.hpp>
#include <boost/range/algorithm/sort.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/algorithm/string/split.hpp>

#include <Inline/BasicTypes.h>
#include <IR/Module.h>
#include <IR/Validate.h>
#include <WAST/WAST.h>
#include <WASM/WASM.h>
#include <Runtime/Runtime.h>

#include <fc/io/fstream.hpp>


#include <fc/log/logger_config.hpp>
#include <boost/thread.hpp>
#include <eos/py_plugin/py_plugin.hpp>
#include <python.h>


#include <eos/chain_api_plugin/chain_api_plugin.hpp>
#include <eos/chain/exceptions.hpp>
#include <fc/io/json.hpp>


using namespace std;
using namespace eos;
using namespace eos::chain;
using namespace eos::utilities;

namespace eos {

class py_plugin_impl {
   public:
};

py_plugin::py_plugin():my(new py_plugin_impl()){}
py_plugin::~py_plugin(){}

void py_plugin::set_program_options(options_description&, options_description& cfg) {
   cfg.add_options()
         ("option-name", bpo::value<string>()->default_value("default value"),
          "Option Description")
         ;
}

void py_plugin::plugin_initialize(const variables_map& options) {
   if(options.count("option-name")) {
      // Handle the option
   }
}

extern "C" void c_printf(const char *s);
extern "C" void PyInit_eosapi();
extern "C" PyObject* PyInit_eostypes_();
extern "C" PyObject* PyInit_wallet();
extern "C" PyObject* PyInit_hello();

void py_thread() {
    PyImport_AppendInittab("hello", PyInit_hello);

    Py_Initialize();
    PyRun_SimpleString("import readline");
    PyInit_eosapi();
    PyInit_eostypes_();
    PyInit_wallet();
    PyRun_SimpleString("import wallet;");
    PyRun_SimpleString("import eosapi;import sys;sys.path.append('./eosd')");
    PyRun_SimpleString("from initeos import *");

    ilog("++++++++++++++py_plugin::plugin_startup");
    //    get_info();
    PyRun_InteractiveLoop(stdin, "<stdin>");
    Py_Finalize();
}

void py_plugin::plugin_startup() {
    boost::thread t{py_thread};
    chain_controller& db = app().get_plugin<chain_plugin>().chain();
}

void py_plugin::plugin_shutdown() {
    ilog("py_plugin::plugin_shutdown()");
    Py_Finalize();
}

}
