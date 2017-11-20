#include <appbase/application.hpp>

#include <eos/account_history_api_plugin/account_history_api_plugin.hpp>
#include <eos/account_history_plugin/account_history_plugin.hpp>
#include <eos/chain_api_plugin/chain_api_plugin.hpp>
#include <eos/chain_plugin/chain_plugin.hpp>
#include <eos/http_plugin/http_plugin.hpp>
#include <eos/net_plugin/net_plugin.hpp>
#include <eos/producer_plugin/producer_plugin.hpp>
#include <eos/py_plugin/py_plugin.hpp>
#include <eos/wallet_api_plugin/wallet_api_plugin.hpp>
#include <eos/wallet_plugin/wallet_plugin.hpp>

#include <fc/exception/exception.hpp>
#include <fc/log/logger_config.hpp>

#include <boost/chrono.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/thread/thread.hpp>


#include <Python.h>

using namespace appbase;
using namespace eos;

static bool init_finished = false;
static bool shutdown_finished = false;

void init_smart_contract();

int eos_main(int argc, char** argv) {
   try {

      app().register_plugin<net_plugin>();
      app().register_plugin<chain_api_plugin>();
      app().register_plugin<producer_plugin>();
      app().register_plugin<account_history_api_plugin>();
      app().register_plugin<wallet_api_plugin>();
      app().register_plugin<py_plugin>();
      if (!app().initialize<chain_plugin, http_plugin, net_plugin, py_plugin>(
              argc, argv)) {
         init_finished = true;
         shutdown_finished = true;
         return -1;
      }

      app().startup();
      init_finished = true;
      app().exec();
   } catch (const fc::exception& e) {
      elog("${e}", ("e", e.to_detail_string()));
   } catch (const boost::exception& e) {
      elog("${e}", ("e", boost::diagnostic_information(e)));
   } catch (const std::exception& e) {
      elog("${e}", ("e", e.what()));
   } catch (...) {
      elog("unknown exception");
   }
   init_finished = true;
   shutdown_finished = true;
}

void quit_app_() {
   app().quit();
   while (!shutdown_finished) {
      boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
   }
}

extern "C" void PyInit_eosapi();
extern "C" PyObject* PyInit_eostypes();
extern "C" PyObject* PyInit_wallet();
//extern "C" PyObject* PyInit_hello();
extern "C" PyObject* PyInit_eostest();
extern "C" PyObject* PyInit_database();
extern "C" PyObject* PyInit_blockchain();
extern "C" PyObject* PyInit_util();

extern "C" PyThreadState *tiny_PyEval_SaveThread(void);
extern "C" void tiny_PyEval_RestoreThread(PyThreadState *tstate);

int main(int argc, char** argv) {
   init_smart_contract();
   _PyGILState_check_enabled = 0;
   Py_Initialize();
   PyEval_InitThreads();

   PyRun_SimpleString("import readline");
   PyInit_wallet();
   PyInit_eosapi();
   PyInit_eostypes();
   PyInit_eostest();
   PyInit_database();
   PyInit_blockchain();
   PyInit_util();

   PyRun_SimpleString("import numpy");
   PyRun_SimpleString("import wallet");
   PyRun_SimpleString("import eosapi;");
   PyRun_SimpleString("import eostest;");
   PyRun_SimpleString("import database;");
   PyRun_SimpleString("from imp import reload;");
   PyRun_SimpleString("eosapi.register_signal_handler()");
   PyRun_SimpleString(
       "import sys;"
        "sys.path.append('../../programs/pyeos');"
        "sys.path.append('../../programs/pyeos/contracts');"
   );

   //Let replay python smart contract get execution
   PyThreadState* state = tiny_PyEval_SaveThread();
   auto thread_ = boost::thread(eos_main, argc, argv);
   while (!init_finished) {
      boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
   }
   tiny_PyEval_RestoreThread(state);
   if (shutdown_finished) {
      Py_Finalize();
      return 0;
   }

   PyRun_SimpleString("import initeos");
   PyRun_SimpleString("from main import chain_controller as ctrl");

   if (app().get_plugin<py_plugin>().interactive) {
      ilog("start interactive python.");
      PyRun_SimpleString("eosapi.register_signal_handler()");
      PyRun_InteractiveLoop(stdin, "<stdin>");
   }

   while (!shutdown_finished) {
      boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
   }

   Py_Finalize();

   return 0;
}
