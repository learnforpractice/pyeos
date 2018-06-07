#include <eosiolib_native/vm_api.h>

static struct vm_api s_api;

void register_vm_api(struct vm_api* api) {
   s_api = *api;
}

struct vm_api* get_vm_api() {
   return &s_api;
}

int setcode(uint64_t account) {
   printf("+++++wasm: setcode\n");
   return 0;
}

int apply(uint64_t receiver, uint64_t account, uint64_t act) {
   printf("+++++wasm: apply\n");
   return 0;
}


