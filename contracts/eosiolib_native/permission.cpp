/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#include <eosiolib/permission.h>


int32_t
check_transaction_authorization( const char* trx_data,     uint32_t trx_size,
                                 const char* pubkeys_data, uint32_t pubkeys_size,
                                 const char* perms_data,   uint32_t perms_size
                               ) {
   return 0;
}

int32_t
check_permission_authorization( account_name account,
                                permission_name permission,
                                const char* pubkeys_data, uint32_t pubkeys_size,
                                const char* perms_data,   uint32_t perms_size,
                                uint64_t delay_us
                              ) {
   return 0;
}

int64_t get_permission_last_used( account_name account, permission_name permission ) {
   return 0;
}

int64_t get_account_creation_time( account_name account ) {
   return 0;
}

