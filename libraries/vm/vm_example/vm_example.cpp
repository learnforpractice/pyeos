#include <stdlib.h>
#include <stdio.h>
#include <eosiolib_native/vm_api.h>

void vm_init(struct vm_api* api) {
   printf("vm_example: init\n");
   vm_register_api(api);
}

void vm_deinit() {
   printf("vm_example: deinit\n");
}

int vm_setcode(uint64_t account) {
   printf("+++++vm_example: setcode\n");
   return 0;
}

int vm_apply(uint64_t receiver, uint64_t account, uint64_t act) {
   printf("+++++vm_example: apply\n");
   return 1;
}

int vm_call(uint64_t account, uint64_t func) {
   printf("+++++vm_example: call\n");
   return 0;
}
