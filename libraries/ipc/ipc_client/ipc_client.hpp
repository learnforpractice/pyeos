#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifndef VM_API_IPC_IPC_CLIENT_CPP_
#define VM_API_IPC_IPC_CLIENT_CPP_

namespace cpp {
class RpcServiceClient;
}

class ipc_client {
public:
   static inline ipc_client& get() {
      static ipc_client* mngr = nullptr;
      if (!mngr) {
         mngr = new ipc_client();
      }
      return *mngr;
   }

   uint64_t get_receiver();

   void require_recipient( uint64_t name );
   void require_auth( uint64_t name );
   void require_auth2( uint64_t name, uint64_t permission );
   void send_inline(char *data, size_t data_len);
   void send_context_free_inline(char *data, size_t data_len);
   uint64_t  publication_time();
   uint32_t get_active_producers( uint64_t* producers, uint32_t buffer_size );

   int32_t db_store_i64(uint64_t scope, uint64_t table, uint64_t payer, uint64_t id,  const void* data, uint32_t len);
   void db_update_i64( int itr, uint64_t payer, const char* buffer, size_t buffer_size );
   void db_remove_i64( int itr );
   int32_t db_get_i64(int32_t iterator, void* data, uint32_t len);
   int32_t db_get_i64_ex( int itr, uint64_t* primary, char* buffer, size_t buffer_size );
   const char* db_get_i64_exex( int itr, size_t* buffer_size );
   int32_t db_next_i64(int32_t iterator, uint64_t* primary);
   int32_t db_previous_i64(int32_t iterator, uint64_t* primary);
   int32_t db_find_i64(uint64_t code, uint64_t scope, uint64_t table, uint64_t id);
   int32_t db_lowerbound_i64(uint64_t code, uint64_t scope, uint64_t table, uint64_t id);
   int32_t db_upperbound_i64(uint64_t code, uint64_t scope, uint64_t table, uint64_t id);
   int32_t db_end_i64(uint64_t code, uint64_t scope, uint64_t table);


   int32_t check_transaction_authorization( const char* trx_data,     uint32_t trx_size,
                                    const char* pubkeys_data, uint32_t pubkeys_size,
                                    const char* perms_data,   uint32_t perms_size
                                  );

   int start();

private:
   cpp::RpcServiceClient* rpcclient = nullptr;

};



#endif /* VM_API_IPC_IPC_CLIENT_CPP_ */
