#include <algorithm>
#include <eosio/chain/apply_context.hpp>
#include <eosio/chain/chain_controller.hpp>
#include <eosio/chain/wasm_interface.hpp>
#include <eosio/chain/micropython_interface.hpp>

#include <eosio/chain/evm_interface.hpp>

#include <eosio/chain/generated_transaction_object.hpp>
#include <eosio/chain/scope_sequence_object.hpp>
#include <eosio/chain/account_object.hpp>

#include <eosio/chain_plugin/chain_plugin.hpp>

#include <appbase/application.hpp>

#include <fc/crypto/xxhash.h>

//mpeoslib.cpp
extern "C" void eosio_assert(int condition, const char* str);

namespace eosio { namespace chain {

char code_buffer[128*1024];


int wasm_call(uint64_t receiver, string& file_name, string& func, vector<char>& args, vector<char>& result) {

   chain_controller& mutable_controller = appbase::app().get_plugin<chain_plugin>().chain();
   apply_context& ctx = apply_context::ctx();

   //FIXME key conflict
   uint64_t id = XXH64(file_name.c_str(), file_name.length(), receiver);

   int itr = ctx.db_find_i64(receiver, receiver, receiver, id);
   if (itr < 0) {
      return -1;
   }

   int code_size = ctx.db_get_i64(itr, &code_buffer[1], sizeof(code_buffer));
   if (code_size <= 0) {
      return -1;
   }

   eosio_assert( code_buffer[1] == 3, "wrong code type");

#if 0
   shared_vector<char> code;
   for (int i=0;i<code_size;i++) {
      code.push_back(code_buffer[i]);
   }

   auto code_id = fc::sha256::hash( code_buffer, (uint32_t)code_size );

   try {
      mutable_controller.get_wasm_interface().apply(code_id, code, ctx);
   } catch ( ... ){throw;}
#endif

}

}}
