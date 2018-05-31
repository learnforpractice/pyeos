#include <appbase/application.hpp>
#include <fc/log/logger_config.hpp>
#include <fc/exception/exception.hpp>
#include <eosio/history_plugin.hpp>
#include <eosio/net_plugin/net_plugin.hpp>
#include <eosio/http_plugin/http_plugin.hpp>
#include <eosio/chain_plugin/chain_plugin.hpp>
#include <eosio/wallet_plugin/wallet_plugin.hpp>
#include <eosio/producer_plugin/producer_plugin.hpp>
#include <eosio/chain_api_plugin/chain_api_plugin.hpp>
#include <eosio/wallet_api_plugin/wallet_api_plugin.hpp>
#include <eosio/history_api_plugin/history_api_plugin.hpp>

#include "config.hpp"

#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp>
#include <boost/exception/diagnostic_information.hpp>


#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <Python.h>

using namespace appbase;
using namespace eosio;

static bool init_finished = false;
static bool shutdown_finished = false;
static bool eos_started = false;

static int g_argc = 0;
static char** g_argv = NULL;

//eosapi.pyx
void py_exit();

extern "C" {
   void init_api();
   PyObject* PyInit_eosapi();
   PyObject* PyInit_wallet();
   PyObject* PyInit_net();

   PyObject* PyInit_eoslib();
//only used in debug
   PyObject* PyInit_eoslib();
   PyObject* PyInit_db();
   PyObject* PyInit_debug();
   PyObject* PyInit_python_contract();
}

bool is_init_finished() {
   return init_finished;
}

void start_eos() {
   try {
      eos_started = true;
      app().set_version(eosio::nodeos::config::version);
      app().register_plugin<history_plugin>();
      app().register_plugin<chain_api_plugin>();
      app().register_plugin<wallet_api_plugin>();

      if(!app().initialize<chain_plugin, http_plugin, net_plugin, producer_plugin>(g_argc, g_argv)) {
         init_finished = true;
         shutdown_finished = true;
         return;
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
   py_exit();
}

void init_console() {
   init_api();

   Py_Initialize();

#ifdef WITH_THREAD
   PyEval_InitThreads();
#endif
   PyRun_SimpleString("import readline");
   PyInit_wallet();
   PyInit_eosapi();
   PyInit_net();

   PyInit_eoslib();
   PyInit_db();
   PyInit_debug();
   PyInit_python_contract();

   PyRun_SimpleString(
       "import sys;"
        "sys.path.append('../../programs/pyeos');"
        "sys.path.append('../../programs/pyeos/contracts');"
   );
   PyRun_SimpleString("import wallet");
   PyRun_SimpleString("import eosapi;");
   PyRun_SimpleString("import eoslib;");
   PyRun_SimpleString("import debug;");
   PyRun_SimpleString("from imp import reload;");
   PyRun_SimpleString("import initeos;initeos.preinit()");

}

extern "C" int eos_main(int argc, char** argv) {
   g_argc = argc;
   g_argv = argv;

   init_console();

   boost::thread t( start_eos );

   while (!init_finished) {
         boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
   }

   //print help
   if (shutdown_finished) {
      return 0;
   }

   PyRun_SimpleString("import initeos");
//   PyRun_SimpleString("initeos.init()");

   if (app().interactive_mode()) {
      PyRun_SimpleString("initeos.start_console()");
   } else {
      while (!shutdown_finished) {
         boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
      }
   }

   return 0;
}


