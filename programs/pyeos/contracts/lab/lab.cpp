#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
extern "C" {
   void sayHello() {
      prints("hello, crypto world\n");
   }
   void apply( uint64_t receiver, uint64_t code, uint64_t action ) {
      auto self = receiver;
      prints("hello,world\n");
      if( code == self ) {
         switch( action ) {
            case N('sayhello'):
                size_t size = action_data_size();
                char msg[size+2];
                msg[size] = '\n';
                msg[size+1] = '\0';
                read_action_data(msg, size);
                prints(msg);
            break;
         }
         sayHello();
         eosio_exit(0);
      }
   }
}

