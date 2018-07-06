/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */

extern "C" {

void unpack_provided_keys( flat_set<public_key_type>& keys, const char* pubkeys_data, size_t pubkeys_size ) {
   keys.clear();
   if( pubkeys_size == 0 ) return;

   keys = fc::raw::unpack<flat_set<public_key_type>>( pubkeys_data, pubkeys_size );
}

void unpack_provided_permissions( flat_set<permission_level>& permissions, const char* perms_data, size_t perms_size ) {
   permissions.clear();
   if( perms_size == 0 ) return;

   permissions = fc::raw::unpack<flat_set<permission_level>>( perms_data, perms_size );
}

int32_t check_transaction_authorization( const char* trx_data,     uint32_t trx_size,
                                 const char* pubkeys_data, uint32_t pubkeys_size,
                                 const char* perms_data,   uint32_t perms_size
                               ) {
   return ipc_client::get().check_transaction_authorization(trx_data, trx_size,
                                    pubkeys_data, pubkeys_size, perms_data, perms_size);

}

int32_t check_permission_authorization( uint64_t account,
                                uint64_t permission,
                                const char* pubkeys_data, uint32_t pubkeys_size,
                                const char* perms_data,   uint32_t perms_size,
                                uint64_t delay_us
                              ) {

   return false;
}

int64_t get_permission_last_used( uint64_t account, uint64_t permission ) {
   return 0;
}

int64_t get_account_creation_time( uint64_t account ) {
   return 0;
}


}
