#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <eosiolib/action.h>
#include <eosiolib_native/vm_api.h>

#include "State.h"
#include "Executive.h"

#include <fc/reflect/reflect.hpp>
#include <fc/io/raw.hpp>

using namespace dev;
using namespace dev::eth;

static struct vm_api s_api;
static dev::eth::State g_s;

uint64_t ethaddr2n(const char* address, int size) {
   eosio_assert(size == Address::size, "wrong address size");
   return Address((byte*)address, Address::ConstructFromPointer);
}

void n2ethaddr(uint64_t n, char* address, int size) {
   eosio_assert(size == Address::size, "wrong address size");
   Address a(n);
   memcpy(address, a.data(), Address::size);
}

void vm_init(struct vm_api* api) {
   api->ethaddr2n = ethaddr2n;
   api->n2ethaddr = n2ethaddr;

   g_s = State();
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
   ts.m_sender = Address(account);
   ts.m_receiveAddress = Address(account);
   ts.m_value = 0;
   ts.m_creation = true;

   size_t size = 0;
   const char* code = get_code(account, &size);
   eosio_assert(size > 0, "bad code");
   ts.m_data.resize(size);
   memcpy(ts.m_data.data(), code, size );

   e.initialize(ts);
   e.execute();
   e.go();
   e.finalize();

   return 1;
}

struct EthTransfer {
   uint64_t from;
   uint64_t to;
   uint64_t value;
   std::vector<uint8_t> data;
};

FC_REFLECT( EthTransfer, (from)(to)(value)(data) )


int vm_apply(uint64_t receiver, uint64_t account, uint64_t act) {
   printf("+++++vm_eth: apply\n");
   if (act != N(ethtransfer)) {
      return 1;
   }

   EnvInfo info;

   Executive e(g_s, info, 0);

   uint32_t size = action_data_size();
   bytes data(size);
   read_action_data( data.data(), data.size() );

   EthTransfer et;
   fc::raw::unpack<EthTransfer>((char*)data.data(), (uint32_t)data.size(), et);

   Transaction ts;
   ts.m_sender = Address(et.from);
   ts.m_receiveAddress = Address(et.to);
   ts.m_value = 0;//et.value;
   ts.m_data = et.data;

   ts.m_creation = false;


   e.initialize(ts);
   e.execute();
   e.go();
   e.finalize();
   return 1;
}

int vm_call(uint64_t account, uint64_t func) {
   return 0;
}
