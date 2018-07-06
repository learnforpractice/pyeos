extern "C" {

uint32_t read_action_data( void* msg, uint32_t buffer_size ) {
   const auto& data = db_api::get().get_action_object().data;
   if( buffer_size == 0 || msg == NULL) return data.size();

   auto copy_size = std::min( (size_t)buffer_size, data.size() );
   memcpy( msg, data.data(), copy_size );
   return copy_size;
}

uint32_t action_data_size() {
   return db_api::get().get_action_object().data.size();
}

uint64_t current_receiver() {
   return db_api::get().get_action_object().receiver;
}

void require_recipient( uint64_t name ) {
   ipc_client::get().require_recipient(name);
}

void require_auth( uint64_t account ) {
   auto& act = db_api::get().get_action_object();
   for( uint32_t i=0; i < act.authorization.size(); i++ ) {
     if( act.authorization[i].actor == account ) {
         #warning FIXME: notify server
//        used_authorizations[i] = true;
        return;
     }
   }
   EOS_ASSERT( false, missing_auth_exception, "missing authority of ${account}", ("account",account));
}

void require_auth2( uint64_t account, uint64_t permission ) {
   auto& act = db_api::get().get_action_object();
   for( uint32_t i=0; i < act.authorization.size(); i++ )
      if( act.authorization[i].actor == account ) {
         if( act.authorization[i].permission == permission ) {
               #warning FIXME: notify server
//            used_authorizations[i] = true;
            return;
         }
      }
   EOS_ASSERT( false, missing_auth_exception, "missing authority of ${account}/${permission}",
               ("account",account)("permission",permission) );
}

bool has_auth( uint64_t account ) {
   auto& act = db_api::get().get_action_object();
   for( const auto& auth : act.authorization ) {
      if( auth.actor == account ) {
         return true;
      }
   }
  return false;
}

bool is_account( uint64_t name ) {
   return db_api::get().is_account(name);
}

void send_inline(char *data, size_t data_len) {
   ipc_client::get().send_inline(data, data_len);
}

void send_context_free_inline(char *data, size_t data_len) {
   ipc_client::get().send_context_free_inline(data, data_len);
}

uint64_t  publication_time() {
   return ipc_client::get().publication_time();
}

}
