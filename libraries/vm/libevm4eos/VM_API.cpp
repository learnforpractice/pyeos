#include <eosiolib_native/vm_api.h>

void evm_checktime() {
   static int counter = 0;
   counter += 1;
   if (counter %10 == 0) {
      get_vm_api()->checktime();
   }
}
