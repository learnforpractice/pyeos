#include <math.h>
#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/action.hpp>
#include <eosio.token/eosio.token.hpp>
#include <eosiolib/print.hpp>

using namespace eosio;


void count() {
   uint64_t code = N(counter);
   uint64_t counter_id = N(counter);
   uint64_t counter;
   int itr = db_find_i64(code, code, code, counter_id);
   if (itr >= 0) {// value exists, update it
       db_get_i64(itr, &counter, sizeof(counter));
       counter += 1;
       db_update_i64(itr, code, &counter, sizeof(counter));
   }
   else {
       counter = 1;
       db_store_i64(code, code, code, counter_id, &counter, sizeof(counter));
   }

   string s("wasm");
   int _itr = db_find_i64(code, code, code, N(msg));
   if (_itr >= 0) {
       db_update_i64(_itr, code, s.c_str(), s.size());
   }
   else {
       db_store_i64(code, code, code, N(msg), s.c_str(), s.size());
   }

}

extern "C" void apply(uint64_t receiver, uint64_t code, uint64_t action) {
   count();
    if (action == N(count)) {
       //count();
    }
}

