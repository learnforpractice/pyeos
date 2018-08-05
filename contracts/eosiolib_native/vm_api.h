#ifndef __WASM_API_H__
#define __WASM_API_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <softfloat_types.h>

#ifndef __EOSIO_CHAIN_TYPES // avoid type conflicts
#include <stdint.h>

#include <eosiolib/types.h>
#include <eosiolib/action.h>
#include <eosiolib/chain.h>
#include <eosiolib/crypto.h>
#include <eosiolib/db.h>
#include <eosiolib/permission.h>
#endif



struct vm_api {
   uint32_t (*read_action_data)( void* msg, uint32_t len );
   uint32_t (*action_data_size)();
   void (*require_recipient)( uint64_t name );
   void (*require_auth)( uint64_t name );
   void (*require_auth2)( uint64_t name, uint64_t permission );
   bool (*has_auth)( uint64_t name );
   bool (*is_account)( uint64_t name );
   void (*send_inline)(const char *serialized_action, size_t size);
   void (*send_context_free_inline)(char *serialized_action, size_t size);
   uint64_t  (*publication_time)();
   uint64_t (*current_receiver)();
   uint32_t (*get_active_producers)( uint64_t* producers, uint32_t datalen );

   void (*assert_sha256)( char* data, uint32_t length, const struct checksum256* hash );
   void (*assert_sha1)( char* data, uint32_t length, const struct checksum160* hash );

   void (*assert_sha512)( char* data, uint32_t length, const struct checksum512* hash );
   void (*assert_ripemd160)( char* data, uint32_t length, const struct checksum160* hash );
   void (*sha256)( char* data, uint32_t length, struct checksum256* hash );
   void (*sha1)( char* data, uint32_t length, struct checksum160* hash );
   void (*sha512)( char* data, uint32_t length, struct checksum512* hash );
   void (*ripemd160)( char* data, uint32_t length, struct checksum160* hash );
   int (*recover_key)( const struct checksum256* digest, const char* sig, size_t siglen, char* pub, size_t publen );
   void (*assert_recover_key)( const struct checksum256* digest, const char* sig, size_t siglen, const char* pub, size_t publen );


   int32_t (*db_store_i64)(uint64_t scope, uint64_t table, uint64_t payer, uint64_t id,  const void* data, uint32_t len);
   int32_t (*db_store_i64_ex)(uint64_t code, uint64_t scope, uint64_t table, uint64_t payer, uint64_t id,  const void* data, uint32_t len);

   void (*db_update_i64)(int32_t iterator, uint64_t payer, const void* data, uint32_t len);
   void (*db_remove_i64)(int32_t iterator);

   void (*db_update_i64_ex)( uint64_t scope, uint64_t payer, uint64_t table, uint64_t id, const char* buffer, size_t buffer_size );
   void (*db_remove_i64_ex)( uint64_t scope, uint64_t payer, uint64_t table, uint64_t id );

   int32_t (*db_get_i64)(int32_t iterator, void* data, uint32_t len);
   int32_t (*db_get_i64_ex)( int itr, uint64_t* primary, char* buffer, size_t buffer_size );
   const char* (*db_get_i64_exex)( int itr, size_t* buffer_size );

   int32_t (*db_next_i64)(int32_t iterator, uint64_t* primary);
   int32_t (*db_previous_i64)(int32_t iterator, uint64_t* primary);
   int32_t (*db_find_i64)(uint64_t code, uint64_t scope, uint64_t table, uint64_t id);
   int32_t (*db_lowerbound_i64)(uint64_t code, uint64_t scope, uint64_t table, uint64_t id);
   int32_t (*db_upperbound_i64)(uint64_t code, uint64_t scope, uint64_t table, uint64_t id);
   int32_t (*db_end_i64)(uint64_t code, uint64_t scope, uint64_t table);

   int32_t (*db_idx64_store)(uint64_t scope, uint64_t table, uint64_t payer, uint64_t id, const uint64_t* secondary);
   void (*db_idx64_update)(int32_t iterator, uint64_t payer, const uint64_t* secondary);
   void (*db_idx64_remove)(int32_t iterator);

   int32_t (*db_idx64_next)(int32_t iterator, uint64_t* primary);
   int32_t (*db_idx64_previous)(int32_t iterator, uint64_t* primary);
   int32_t (*db_idx64_find_primary)(uint64_t code, uint64_t scope, uint64_t table, uint64_t* secondary, uint64_t primary);
   int32_t (*db_idx64_find_secondary)(uint64_t code, uint64_t scope, uint64_t table, const uint64_t* secondary, uint64_t* primary);
   int32_t (*db_idx64_lowerbound)(uint64_t code, uint64_t scope, uint64_t table, uint64_t* secondary, uint64_t* primary);
   int32_t (*db_idx64_upperbound)(uint64_t code, uint64_t scope, uint64_t table, uint64_t* secondary, uint64_t* primary);
   int32_t (*db_idx64_end)(uint64_t code, uint64_t scope, uint64_t table);
   int32_t (*db_idx128_store)(uint64_t scope, uint64_t table, uint64_t payer, uint64_t id, const uint128_t* secondary);

   void (*db_idx128_update)(int32_t iterator, uint64_t payer, const uint128_t* secondary);
   void (*db_idx128_remove)(int32_t iterator);
   int32_t (*db_idx128_next)(int32_t iterator, uint64_t* primary);
   int32_t (*db_idx128_previous)(int32_t iterator, uint64_t* primary);
   int32_t (*db_idx128_find_primary)(uint64_t code, uint64_t scope, uint64_t table, uint128_t* secondary, uint64_t primary);
   int32_t (*db_idx128_find_secondary)(uint64_t code, uint64_t scope, uint64_t table, const uint128_t* secondary, uint64_t* primary);
   int32_t (*db_idx128_lowerbound)(uint64_t code, uint64_t scope, uint64_t table, uint128_t* secondary, uint64_t* primary);
   int32_t (*db_idx128_upperbound)(uint64_t code, uint64_t scope, uint64_t table, uint128_t* secondary, uint64_t* primary);

   int32_t (*db_idx128_end)(uint64_t code, uint64_t scope, uint64_t table);
   int32_t (*db_idx256_store)(uint64_t scope, uint64_t table, uint64_t payer, uint64_t id, const uint128_t* data, size_t data_len );
   void (*db_idx256_update)(int32_t iterator, uint64_t payer, const void* data, size_t data_len);
   void (*db_idx256_remove)(int32_t iterator);
   int32_t (*db_idx256_next)(int32_t iterator, uint64_t* primary);

   int32_t (*db_idx256_previous)(int32_t iterator, uint64_t* primary);
   int32_t (*db_idx256_find_primary)(uint64_t code, uint64_t scope, uint64_t table, uint128_t* data, size_t data_len, uint64_t primary);
   int32_t (*db_idx256_find_secondary)(uint64_t code, uint64_t scope, uint64_t table, const uint128_t* data, size_t data_len, uint64_t* primary);
   int32_t (*db_idx256_lowerbound)(uint64_t code, uint64_t scope, uint64_t table, uint128_t* data, size_t data_len, uint64_t* primary);
   int32_t (*db_idx256_upperbound)(uint64_t code, uint64_t scope, uint64_t table, uint128_t* data, size_t data_len, uint64_t* primary);
   int32_t (*db_idx256_end)(uint64_t code, uint64_t scope, uint64_t table);
   int32_t (*db_idx_double_store)(uint64_t scope, uint64_t table, uint64_t payer, uint64_t id, const float64_t* secondary);
   void (*db_idx_double_update)(int32_t iterator, uint64_t payer, const float64_t* secondary);
   void (*db_idx_double_remove)(int32_t iterator);
   int32_t (*db_idx_double_next)(int32_t iterator, uint64_t* primary);
   int32_t (*db_idx_double_previous)(int32_t iterator, uint64_t* primary);
   int32_t (*db_idx_double_find_primary)(uint64_t code, uint64_t scope, uint64_t table, float64_t* secondary, uint64_t primary);
   int32_t (*db_idx_double_find_secondary)(uint64_t code, uint64_t scope, uint64_t table, const float64_t* secondary, uint64_t* primary);
   int32_t (*db_idx_double_lowerbound)(uint64_t code, uint64_t scope, uint64_t table, float64_t* secondary, uint64_t* primary);
   int32_t (*db_idx_double_upperbound)(uint64_t code, uint64_t scope, uint64_t table, float64_t* secondary, uint64_t* primary);
   int32_t (*db_idx_double_end)(uint64_t code, uint64_t scope, uint64_t table);
   int32_t (*db_idx_long_double_store)(uint64_t scope, uint64_t table, uint64_t payer, uint64_t id, const float128_t* secondary);
   void (*db_idx_long_double_update)(int32_t iterator, uint64_t payer, const float128_t* secondary);
   void (*db_idx_long_double_remove)(int32_t iterator);
   int32_t (*db_idx_long_double_next)(int32_t iterator, uint64_t* primary);
   int32_t (*db_idx_long_double_previous)(int32_t iterator, uint64_t* primary);
   int32_t (*db_idx_long_double_find_primary)(uint64_t code, uint64_t scope, uint64_t table, float128_t* secondary, uint64_t primary);
   int32_t (*db_idx_long_double_find_secondary)(uint64_t code, uint64_t scope, uint64_t table, const float128_t* secondary, uint64_t* primary);
   int32_t (*db_idx_long_double_lowerbound)(uint64_t code, uint64_t scope, uint64_t table, float128_t* secondary, uint64_t* primary);
   int32_t (*db_idx_long_double_upperbound)(uint64_t code, uint64_t scope, uint64_t table, float128_t* secondary, uint64_t* primary);
   int32_t (*db_idx_long_double_end)(uint64_t code, uint64_t scope, uint64_t table);


   int32_t (*check_transaction_authorization)( const char* trx_data,     uint32_t trx_size,
                                    const char* pubkeys_data, uint32_t pubkeys_size,
                                    const char* perms_data,   uint32_t perms_size
                                  );
   int32_t (*check_permission_authorization)( uint64_t account,
                                   uint64_t permission,
                                   const char* pubkeys_data, uint32_t pubkeys_size,
                                   const char* perms_data,   uint32_t perms_size,
                                   uint64_t delay_us
                                 );
   int64_t (*get_permission_last_used)( uint64_t account, uint64_t permission );
   int64_t (*get_account_creation_time)( uint64_t account );



   void (*prints)( const char* cstr );
   void (*prints_l)( const char* cstr, uint32_t len);
   void (*printi)( int64_t value );
   void (*printui)( uint64_t value );
   void (*printi128)( const int128_t* value );
   void (*printui128)( const uint128_t* value );
   void (*printsf)(float value);
   void (*printdf)(double value);
   void (*printqf)(const float128_t* value);
   void (*printn)( uint64_t name );
   void (*printhex)( const void* data, uint32_t datalen );

   void (*set_resource_limits)( uint64_t account, int64_t ram_bytes, int64_t net_weight, int64_t cpu_weight );
   void (*get_resource_limits)( uint64_t account, int64_t* ram_bytes, int64_t* net_weight, int64_t* cpu_weight );
   int64_t (*set_proposed_producers)( char *producer_data, uint32_t producer_data_size );
   bool (*is_privileged)( uint64_t account );
   void (*set_privileged)( uint64_t account, bool is_priv );
   void (*set_blockchain_parameters_packed)(char* data, uint32_t datalen);
   uint32_t (*get_blockchain_parameters_packed)(char* data, uint32_t datalen);
   void (*activate_feature)( int64_t f );

   void (*eosio_abort)();
   void (*eosio_assert)( uint32_t test, const char* msg );
   void (*eosio_assert_message)( uint32_t test, const char* msg, uint32_t msg_len );
   void (*eosio_assert_code)( uint32_t test, uint64_t code );
   void (*eosio_exit)( int32_t code );
   uint64_t  (*current_time)();
   uint32_t  (*now)();

   void (*checktime)();
   void (*check_context_free)(bool context_free);
   bool (*contracts_console)();

   void (*send_deferred)(const uint128_t* sender_id, uint64_t payer, const char *serialized_transaction, size_t size, uint32_t replace_existing);
   int (*cancel_deferred)(const uint128_t* sender_id);

   size_t (*read_transaction)(char *buffer, size_t size);
   size_t (*transaction_size)();
   int (*tapos_block_num)();
   int (*tapos_block_prefix)();
   uint32_t (*expiration)();
   int (*get_action)( uint32_t type, uint32_t index, char* buff, size_t size );

   void (*assert_privileged)();
   void (*assert_context_free)();
   int (*get_context_free_data)( uint32_t index, char* buff, size_t size );

   const char* (*get_code)( uint64_t receiver, size_t* size );
   void (*set_code)(uint64_t user_account, int vm_type, uint64_t last_code_update, char *code_version, int version_size, char* code, int code_size);
   int (*get_code_id)( uint64_t account, char* code_id, size_t size );

   void (*rodb_remove_i64)( int32_t itr );
   int32_t (*rodb_get_i64)( int32_t itr, char* buffer, size_t buffer_size );
   int32_t (*rodb_get_i64_ex)( int iterator, uint64_t* primary, char* buffer, size_t buffer_size );
   const char* (*rodb_get_i64_exex)( int itr, size_t* buffer_size );

   int32_t (*rodb_next_i64)( int32_t itr, uint64_t* primary );
   int32_t (*rodb_previous_i64)( int32_t itr, uint64_t* primary );
   int32_t (*rodb_find_i64)( uint64_t code, uint64_t scope, uint64_t table, uint64_t id );
   int32_t (*rodb_lowerbound_i64)( uint64_t code, uint64_t scope, uint64_t table, uint64_t id );
   int32_t (*rodb_upperbound_i64)( uint64_t code, uint64_t scope, uint64_t table, uint64_t id );
   int32_t (*rodb_end_i64)( uint64_t code, uint64_t scope, uint64_t table );


   int (*split_path)(const char* str_path, char *path1, size_t path1_size, char *path2, size_t path2_size);
   uint64_t (*get_action_account)();
   uint64_t (*string_to_uint64)(const char* str);
   int32_t (*uint64_to_string)(uint64_t n, char* out, int size);
   uint64_t (*string_to_symbol)(uint8_t precision, const char* str);

   void (*resume_billing_timer)();
   void (*pause_billing_timer)();

   uint64_t (*wasm_call)(const char*func, uint64_t* args , int argc);

   int (*call_set_args)(const char* args , int len);
   int (*call_get_args)(char* args , int len);

   int (*call)(uint64_t account, uint64_t func);

   int (*call_set_results)(const char* result , int len);
   int (*call_get_results)(char* result , int len);

   int (*has_option)(const char* _option);
   int (*get_option)(const char* option, char *result, int size);
   int (*app_init_finished)();

   int (*run_mode)(); // 0 for server, 1 for client

   char reserved[sizeof(char*)*128]; //for forward compatibility
};

typedef void (*fn_register_vm_api)(struct vm_api* api);

typedef int (*fn_setcode)(uint64_t account);
typedef int (*fn_apply)(uint64_t receiver, uint64_t account, uint64_t act);
typedef int (*fn_call)(uint64_t account, uint64_t func);

typedef void (*fn_vm_init)(struct vm_api* api);
typedef void (*fn_vm_deinit)();
typedef int (*fn_preload)(uint64_t account);
typedef int (*fn_unload)(uint64_t account);


void vm_init(struct vm_api* api);
void vm_deinit();
void vm_register_api(struct vm_api* api);

int vm_setcode(uint64_t account);
int vm_apply(uint64_t receiver, uint64_t account, uint64_t act);
int vm_call(uint64_t account, uint64_t func);

int vm_preload(uint64_t account);
int vm_unload(uint64_t account);

struct vm_api* get_vm_api();

uint64_t wasm_call(const char* act, uint64_t* args, int argc);


int has_option(const char* _option);
int get_option(const char* option, char *result, int size);

#ifdef __cplusplus
}
#endif

#endif
