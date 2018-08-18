#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <eosiolib_native/vm_api.h>
#include <VMFactory.h>

#include "ExtVM.h"


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

int vm_apply(uint64_t receiver, uint64_t account, uint64_t act) {
   printf("+++++vm_eth: apply\n");
   auto vm = VMFactory::create();
   EnvInfo info;
   Address recv(0);
   Address sender(0);
   Address origin(0);
   dev::bytesConstRef data;
   dev::bytesConstRef code;
   h256 codeHash(0);
   ExtVM ctx(info, recv, sender, origin, 0, 0, data, code, codeHash, 0, true, false);

   auto onOp = [&](uint64_t step, uint64_t PC, Instruction inst, dev::bigint m, dev::bigint gasCost, dev::bigint gas, dev::eth::VMFace const* evm, ExtVMFace const* extVM) {
//      std::cout << "++++++gasCost: " << gasCost << "\n";
#if 0
      if (mode == Mode::Statistics)
      {
         counts[(byte)inst].first++;
         counts[(byte)inst].second += gasCost;
         total++;
         if (m > 0)
            memTotal = m;
      }
      else if (mode == Mode::Trace)
         st(step, PC, inst, m, gasCost, gas, evm, extVM);
#endif
   };
   u256 gas(0);
   auto out = vm->exec(gas, ctx, onOp);
   out.toVector();
   return 0;
}

int vm_call(uint64_t account, uint64_t func) {
   return 0;
}
