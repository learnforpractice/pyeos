#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <eosiolib/action.h>
#include <eosiolib_native/vm_api.h>

#include "State.h"
#include "Executive.h"

using namespace dev;
using namespace dev::eth;

static struct vm_api s_api;
static dev::eth::State g_s;

void vm_init(struct vm_api* api) {
   s_api = *api;
   vm_register_api(api);
}

void vm_deinit() {
   printf("vm_eth finalize\n");
}

int vm_setcode(uint64_t account) {
   printf("+++++vm_eth: setcode\n");
   EnvInfo info;

   Executive e(g_s, info, 0);
   Transaction ts;
   ts.m_sender = Address(0);
   ts.m_receiveAddress = Address(account);
   ts.m_value = 0;
   ts.m_creation = false;

   uint32_t size = action_data_size();
   ts.m_data.resize(size);
   read_action_data( ts.m_data.data(), size );

   e.initialize(ts);
   e.execute();
   e.go();
   e.finalize();

   return 1;
}


int vm_apply(uint64_t receiver, uint64_t account, uint64_t act) {
   printf("+++++vm_eth: apply\n");
   EnvInfo info;

   Executive e(g_s, info, 0);
   Transaction ts;
   ts.m_sender = Address(0);
   ts.m_receiveAddress = Address(receiver);
   ts.m_value = 0;
   ts.m_creation = false;

   uint32_t size = action_data_size();
   ts.m_data.resize(size);
   read_action_data( ts.m_data.data(), size );

   e.initialize(ts);
   e.execute();
   e.go();
   e.finalize();
   return 1;
}

int vm_call(uint64_t account, uint64_t func) {
   return 0;
}
