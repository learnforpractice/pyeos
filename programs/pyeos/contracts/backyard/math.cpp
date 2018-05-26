#include <eosiolib/print.hpp>
#include <stdint.h>
void eosio_assert(int cond, char* msg);
extern "C" uint64_t add(uint64_t a, uint64_t b) {
   printi(a+b);
//   eosio_assert(0, "hello, world");
   return a+b;
}
