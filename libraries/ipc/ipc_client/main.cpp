#include <vm_manager.hpp>
#include "ipc_client.hpp"


namespace eosio {
namespace chain {
   void vm_manager_init();
}
}

int main(int argc, char** argv) {
   wlog("hello,world from ipc client");
   eosio::chain::vm_manager_init();
   ipc_client::get().start();
   return 0;
}


