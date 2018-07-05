#include <eosiolib_native/vm_api.h>

void vm_init() {

}

void vm_deinit() {

}

void vm_register_api(struct vm_api* api) {

}

int vm_setcode(uint64_t account) {
   return -1;
}

int vm_apply(uint64_t receiver, uint64_t account, uint64_t act) {
   return -1;
}

uint64_t vm_call(const char* act, uint64_t* args, int argc) {
   return 0;
}

int vm_preload(uint64_t account) {
   return -1;
}

int vm_unload(uint64_t account) {
   return -1;
}
