
#include "native_interface.hpp"


static struct vm_api* s_api;

void vm_init(struct vm_api* api) {
   s_api = api;
   vm_register_api(api);
}

void vm_deinit() {
   printf("vm_native finalize\n");
}

struct vm_api* get_vm_api() {
   return s_api;
}

int vm_setcode(uint64_t account) {
   printf("+++++vm_native: setcode\n");
   return 0;
}

int vm_apply(uint64_t receiver, uint64_t account, uint64_t act) {
   return eosio::chain::native_interface::get().apply(receiver, account, act);
}

int vm_call(uint64_t account, uint64_t func) {
   return 0;
}
