#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>

extern "C" {
   void sayHello() {
#if 0
      int n = 0;
      for(int i=0;i<1000;i++) {
         n += i;
      }
      printui(n);
#endif
      uint64_t code = N(actiontest);
      uint64_t id = N(name);

      char buffer[128];
      size_t size = action_data_size();
      read_action_data(buffer, size);

      int itr = db_find_i64(code, code, code, id);
      if (itr >= 0) {
          db_get_i64(itr, buffer, sizeof(buffer));
          db_update_i64(itr, code, buffer, size);
      }
      else {
         db_store_i64(code, code, code, id, buffer, size);
      }
      prints("hello,world\n");
   }

   void apply( uint64_t receiver, uint64_t code, uint64_t action ) {
      auto self = receiver;
      if( code == self ) {
         switch( action ) {
            case N(sayhello):
               sayHello();
            break;
         }
         eosio_exit(0);
      }
   }
}
