#include <vm_manager.hpp>
#include "ipc_client.hpp"


namespace eosio {
namespace chain {
   void vm_manager_init(int vm_type);
}
}

int main(int argc, char** argv) {
   const char *ipc_path;
   if (argc != 3) {
      wlog("usage: ipc_client <ipc path> <vm index>");
      return -1;
   }

   int vm_type = strtol(argv[2],NULL, 10);
   ipc_path = (const char*)argv[1];

   eosio::chain::vm_manager_init(vm_type);

   wlog("ipc path ${n}", ("n", ipc_path));
   ipc_client::get().start(ipc_path);
   return 0;
}


