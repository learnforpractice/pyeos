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
   ipc_manager::get().require_recipient(name);
}

void require_auth( uint64_t name ) {
   ipc_manager::get().require_auth(name);
}

void require_auth2( uint64_t name, uint64_t permission ) {
   ipc_manager::get().require_auth2(name, permission);
}

bool has_auth( uint64_t account ) {
   for( const auto& auth : db_api::get().get_action_object().authorization )
     if( auth.actor == account )
        return true;
  return false;
}

bool is_account( uint64_t name ) {
   return db_api::get().is_account(name);
}

void send_inline(char *data, size_t data_len) {
   ipc_manager::get().send_inline(data, data_len);
}

void send_context_free_inline(char *data, size_t data_len) {
   ipc_manager::get().send_context_free_inline(data, data_len);
}

uint64_t  publication_time() {
   return ipc_manager::get().publication_time();
}

}
