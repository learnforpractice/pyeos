#include <math.h>
#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/action.hpp>
#include <eosio.token/eosio.token.hpp>
#include <eosiolib/print.hpp>

using namespace eosio;

extern "C" {
void sayHello() {
   prints("hello, crypto world\n");
}

uint64_t mypow(uint64_t base, uint64_t power) {
   double ret = pow(*(double*)&base, *(double*)&power);
   return *(uint64_t*)&ret;
}

void apply( uint64_t receiver, uint64_t code, uint64_t action ) {
   auto self = receiver;
//   char* a = (char*)malloc(1024*1024*1024);
//   a[10000] = 0;
//   print("+++++++abc:", uint64_t(a), "\n");
   if( code == self ) {
      switch( action ) {
         case N(sayhello):
            size_t size = action_data_size();
            if (size > 128) {
               size = 128;
            }
            printui(mypow(10.1, 2));
            char msg[size+2];
            msg[size] = '\n';
            msg[size+1] = '\0';
            read_action_data(msg, size);
            prints(msg);
         break;
      }
      eosio_exit(0);
   }
}
}

