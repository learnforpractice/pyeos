
#include "native_interface.hpp"
#include <string>

static struct vm_api* s_api;

static uint64_t s_debug_account = -1;
static std::string s_debug_contract_path;

void vm_set_debug_contract(uint64_t account, const char* path) {
   s_debug_account = account;
   s_debug_contract_path = path;
}

const char* vm_get_debug_contract(uint64_t* account) {
   *account = s_debug_account;
   if (s_debug_contract_path.size()) {
      return s_debug_contract_path.c_str();
   }
   return nullptr;
}

void vm_init(struct vm_api* api) {
   api->vm_set_debug_contract = vm_set_debug_contract,
   api->vm_get_debug_contract = vm_get_debug_contract,
   s_api = api;
}

void vm_deinit() {
   printf("vm_native finalize\n");
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
