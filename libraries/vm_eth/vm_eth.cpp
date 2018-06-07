#include <eosiolib_native/vm_api.h>

static struct vm_api s_api;

void register_vm_api(struct vm_api* api) {
   s_api = *api;
}

struct vm_api* get_vm_api() {
   return &s_api;
}

void setcode(uint64_t account) {
   printf("+++++micropython: setcode");
}

void apply(uint64_t receiver, uint64_t account, uint64_t act) {
   printf("+++++micropython: apply");
}

