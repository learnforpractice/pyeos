/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */


int32_t
check_transaction_authorization( const char* trx_data,     uint32_t trx_size,
                                 const char* pubkeys_data, uint32_t pubkeys_size,
                                 const char* perms_data,   uint32_t perms_size
                               ) {
   array_ptr<char> trx_ptr((char*)trx_data);
   array_ptr<char> pubkeys_ptr((char*)pubkeys_data);
   array_ptr<char> perms_ptr((char*)perms_data);

   return permission_api(ctx(), false).check_transaction_authorization(trx_ptr, trx_size,
         pubkeys_ptr, pubkeys_size,
         perms_ptr, perms_size);
}

int32_t
check_permission_authorization( account_name account,
                                permission_name permission,
                                const char* pubkeys_data, uint32_t pubkeys_size,
                                const char* perms_data,   uint32_t perms_size,
                                uint64_t delay_us
                              ) {
   array_ptr<char> pubkeys_ptr((char*)pubkeys_data);
   array_ptr<char> perms_ptr((char*)perms_data);

   return permission_api(ctx(), false).check_permission_authorization( account, permission,
                                              pubkeys_ptr, pubkeys_size,
                                              perms_ptr,   perms_size,
                                              delay_us
                                            );
}

int64_t get_permission_last_used( account_name account, permission_name permission ) {
   return permission_api(ctx(), false).get_permission_last_used(account, permission);
}

int64_t get_account_creation_time( account_name account ) {
   return permission_api(ctx(), false).get_account_creation_time(account);;
}

