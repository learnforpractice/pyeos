#include <vm_manager.hpp>
#include "ipc_client.hpp"


namespace eosio {
namespace chain {
   void vm_manager_init();
}
}

int main(int argc, char** argv) {
   eosio::chain::vm_manager_init();
   const char *ipc_path;
   if (argc >= 2) {
      ipc_path = (const char*)argv[1];
   } else {
      ipc_path ="/tmp/pyeos.ipc";
   }
   wlog("ipc path ${n}", ("n", ipc_path));
   ipc_client::get().start(ipc_path);
   return 0;
}


