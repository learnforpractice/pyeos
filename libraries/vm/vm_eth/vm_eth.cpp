#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <eosiolib_native/vm_api.h>

#include "State.h"
#include "Executive.h"

using namespace dev;
using namespace dev::eth;

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

static dev::eth::State g_s;

int vm_apply(uint64_t receiver, uint64_t account, uint64_t act) {
   printf("+++++vm_eth: apply\n");
   EnvInfo info;
   Address recv(0);
   Address sender(0);
   Address origin(0);
   dev::bytesConstRef data;
   dev::bytesConstRef code;
   h256 codeHash(0);
   bytes output;
//   ExtVM ctx(_s, info, recv, sender, origin, 0, 0, data, code, codeHash, 0, true, false);

   Executive exec(g_s, info, 0);
   exec.execute();
   exec.go(output);
   exec.finalize();
   return 1;
}

int vm_call(uint64_t account, uint64_t func) {
   return 0;
}
