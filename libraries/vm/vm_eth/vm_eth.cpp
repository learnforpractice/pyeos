#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <eosiolib_native/vm_api.h>
#include <VMFactory.h>

static struct vm_api s_api;

void vm_init(struct vm_api* api) {
   s_api = *api;
}

void vm_deinit() {
   printf("vm_eth finalize\n");
}

struct vm_api* get_vm_api() {
   return &s_api;
}

int vm_setcode(uint64_t account) {
   printf("+++++vm_eth: setcode\n");
   return 0;
}

int vm_apply(uint64_t receiver, uint64_t account, uint64_t act) {
   printf("+++++vm_eth: apply\n");
   auto vm = dev::eth::VMFactory::create();
   return 0;
}

int vm_call(uint64_t account, uint64_t func) {
   return 0;
}
