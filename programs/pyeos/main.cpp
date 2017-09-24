#include <appbase/application.hpp>

#include <eos/producer_plugin/producer_plugin.hpp>
#include <eos/chain_plugin/chain_plugin.hpp>
#include <eos/http_plugin/http_plugin.hpp>
#include <eos/chain_api_plugin/chain_api_plugin.hpp>
#include <eos/net_plugin/net_plugin.hpp>
#include <eos/account_history_plugin/account_history_plugin.hpp>
#include <eos/account_history_api_plugin/account_history_api_plugin.hpp>
#include <eos/py_plugin/py_plugin.hpp>
#include <eos/wallet_plugin/wallet_plugin.hpp>
#include <eos/wallet_api_plugin/wallet_api_plugin.hpp>

#include <fc/log/logger_config.hpp>
#include <fc/exception/exception.hpp>

#include <boost/exception/diagnostic_information.hpp>

#include <Python.h>

using namespace appbase;
using namespace eos;

int eos_thread(int argc, char** argv) {
   try {
      app().register_plugin<net_plugin>();
      app().register_plugin<chain_api_plugin>();
      app().register_plugin<http_plugin>();
      app().register_plugin<producer_plugin>();
      app().register_plugin<chain_plugin>();
      app().register_plugin<account_history_plugin>();
      app().register_plugin<account_history_api_plugin>();
      app().register_plugin<wallet_plugin>();
      app().register_plugin<wallet_api_plugin>();
      app().register_plugin<py_plugin>();
      if(!app().initialize<chain_plugin, http_plugin, net_plugin,account_history_api_plugin,wallet_plugin,py_plugin>(argc, argv))
         return -1;
      app().startup();
      app().exec();
   } catch (const fc::exception& e) {
      elog("${e}", ("e",e.to_detail_string()));
   } catch (const boost::exception& e) {
      elog("${e}", ("e",boost::diagnostic_information(e)));
   } catch (const std::exception& e) {
      elog("${e}", ("e",e.what()));
   } catch (...) {
      elog("unknown exception");
   }
}

extern "C" void PyInit_eosapi();
extern "C" PyObject* PyInit_eostypes_();
extern "C" PyObject* PyInit_wallet();
extern "C" PyObject* PyInit_hello();
extern "C" PyObject* PyInit_python_contract();
extern "C" PyObject* PyInit_eoslib();

int main(int argc, char** argv)
{
//   Py_InitializeEx(0);
   Py_Initialize();
   PyEval_InitThreads();

   PyRun_SimpleString("import readline");
   PyInit_eosapi();
   PyInit_eostypes_();
   PyInit_wallet();
   PyInit_python_contract();
   PyInit_eoslib();

   PyRun_SimpleString("import wallet");
   PyRun_SimpleString("import eoslib");
   PyRun_SimpleString("import eosapi;import sys;sys.path.append('../../programs/pyeos')");
   PyRun_SimpleString("from initeos import *");


//   boost::thread t{eos_thread};
   auto thread_ = boost::thread(eos_thread,argc,argv);

   PyRun_InteractiveLoop(stdin, "<stdin>");
   Py_Finalize();

   return 0;
}

