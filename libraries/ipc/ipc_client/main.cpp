#include <vm_manager.hpp>
#include "ipc_client.hpp"

#include <unistd.h>

#include <appbase/application.hpp>

namespace eosio {
namespace chain {
   void vm_manager_init(int vm_type);
}
}

using namespace appbase;

int main(int argc, char** argv) {

   appbase::app().initialize<>(argc, argv);
   string _vm_type = app().get_option("vm-index");
   string _ipc_dir = app().get_option("ipc-dir");

   int vm_type = strtol(_vm_type.c_str(),NULL, 10);

   eosio::chain::vm_manager_init(vm_type);

   wlog("ipc client ${n1} started, ipc path ${n2}", ("n1", getpid())("n2", _ipc_dir));
   ipc_client::get().start(_ipc_dir.c_str());
   return 0;
}


