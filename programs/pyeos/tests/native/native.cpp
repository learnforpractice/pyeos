#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/db.h>
#include <eosiolib/memory.h>

extern "C" {

void apply( uint64_t receiver, uint64_t code, uint64_t action ) {
   auto self = receiver;
   if( code == self ) {
      switch( action ) {
         case N(deploy):
            size_t size = action_data_size();
            char *msg = (char*)malloc(size);
            read_action_data(msg, size);

         break;
      }
      eosio_exit(0);
   }
}


}

