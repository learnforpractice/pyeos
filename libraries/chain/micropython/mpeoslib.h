/*
 * eoslib_.hpp
 *
 *  Created on: Sep 13, 2017
 *      Author: newworld
 */

#ifndef MPEOSLIB__HPP_
#define MPEOSLIB__HPP_

#ifdef __cplusplus
   extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "py/objlist.h"
#include "py/objstringio.h"
#include "py/runtime.h"
#include "py/stream.h"
#include "py/parse.h"

typedef __uint128_t uint128_t;

struct mp_action {
   uint64_t                   account;
   uint64_t                   name;
   uint64_t*                  auth;
   size_t                     auth_len;
   unsigned char *            data;
   size_t                     data_len;
};

struct mp_transaction {
   struct mp_action**      context_free_actions;
   size_t                 free_actions_len;
   struct mp_action**      actions;
   size_t                 actions_len;

   uint32_t               expiration;   ///< the time at which a transaction expires
   uint16_t               ref_block_num       ; ///< specifies a block num in the last 2^16 blocks.
   uint32_t               ref_block_prefix    ; ///< specifies the lower 32 bits of the blockid at get_ref_blocknum
   uint32_t               max_net_usage_words ; /// upper limit on total network bandwidth (in 8 byte words) billed for this transaction
   uint32_t               delay_sec           ; /// number of seconds to delay this transaction for during which it may be canceled.
};

void send_inline( struct mp_action* mp_act );
void send_context_free_inline(struct mp_action* mp_act);
void send_deferred( uint128_t sender_id, uint64_t payer, struct mp_transaction* mp_trx );
void cancel_deferred( uint128_t  val );

uint32_t now();
void abort_();
void eosio_assert(int condition, const char* str);
//class crypto_api
void assert_recover_key( const char* data, size_t data_len,
                  const char* sig, size_t siglen,
                  const char* pub, size_t publen );
mp_obj_t recover_key(const char* data, size_t size, const char* sig, size_t siglen );
void assert_sha256(const char* data, size_t datalen, const char* hash, size_t hash_len);
void assert_sha1(const char* data, size_t datalen, const char* hash, size_t hash_len);
void assert_sha512(const char* data, size_t datalen, const char* hash, size_t hash_len);
void assert_ripemd160(const char* data, size_t datalen, const char* hash, size_t hash_len);
mp_obj_t sha1_(const char* data, size_t datalen);
mp_obj_t sha256_(const char* data, size_t datalen);
mp_obj_t sha512_(const char* data, size_t datalen);
mp_obj_t ripemd160_(const char* data, size_t datalen);



uint64_t string_to_uint64_(const char* str);
//mp_obj_t uint64_to_string_(uint64_t n);

#define Name uint64_t

mp_obj_t pack_(const char* str, int nsize);
mp_obj_t unpack_(const char* str, int nsize);

//int get_account_balance_(Name account, uint64_t& eos_balance, uint64_t& staked_balance, uint32_t& unstaking_balance, uint32_t& last_unstaking_time);
mp_obj_t get_account_balance_(Name account);
uint64_t get_active_producers_();
mp_obj_t  sha256_(const char* str, size_t nsize);

int read_transaction( char* data, size_t data_len );
int transaction_size();
int expiration();
int tapos_block_num();
int tapos_block_prefix();
int get_action( uint32_t type, uint32_t index, char* buffer, size_t buffer_size );


void require_auth(uint64_t account);
void require_auth_ex(uint64_t account, uint64_t permission);
int is_account(uint64_t account);
void require_recipient(uint64_t account);


int read_action(char* memory, size_t size);
int action_size();
uint64_t current_receiver();
uint64_t publication_time();
uint64_t current_sender();

uint64_t get_action_account();
//uint64_t string_to_symbol( uint8_t precision, const char* str );



int db_store_i64( uint64_t scope, uint64_t table, uint64_t payer, uint64_t id, const char* buffer, size_t buffer_size );
void db_update_i64( int itr, uint64_t payer, const char* buffer, size_t buffer_size );
void db_remove_i64( int itr );
int db_get_i64( int itr, char* buffer, size_t buffer_size );
int db_get_i64_ex( int itr, uint64_t* primary, char* buffer, size_t buffer_size );
int db_next_i64( int itr, uint64_t* primary );
int db_previous_i64( int itr, uint64_t* primary );
int db_find_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id );
int db_lowerbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id );
int db_upperbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id );
int db_end_i64( uint64_t code, uint64_t scope, uint64_t table );


#define DB_API_METHOD_WRAPPERS_SIMPLE_SECONDARY_DEF(IDX, TYPE)\
      int db_##IDX##_store( uint64_t scope, uint64_t table, uint64_t payer, uint64_t id, const char* secondary , size_t len); \
      void db_##IDX##_update( int iterator, uint64_t payer, const char* secondary , size_t len ); \
      void db_##IDX##_remove( int iterator ); \
      int db_##IDX##_find_secondary( uint64_t code, uint64_t scope, uint64_t table, const char* secondary , size_t len, uint64_t* primary ); \
      int db_##IDX##_find_primary( uint64_t code, uint64_t scope, uint64_t table, char* secondary , size_t len, uint64_t primary ); \
      int db_##IDX##_lowerbound( uint64_t code, uint64_t scope, uint64_t table, char* secondary , size_t len, uint64_t* primary ); \
      int db_##IDX##_upperbound( uint64_t code, uint64_t scope, uint64_t table, char* secondary , size_t len, uint64_t* primary ); \
      int db_##IDX##_end( uint64_t code, uint64_t scope, uint64_t table ); \
      int db_##IDX##_next( int iterator, uint64_t* primary  ); \
      int db_##IDX##_previous( int iterator, uint64_t* primary );

#define DB_API_METHOD_WRAPPERS_ARRAY_SECONDARY_DEF(IDX, ARR_SIZE, ARR_ELEMENT_TYPE)\
      int db_##IDX##_store( uint64_t scope, uint64_t table, uint64_t payer, uint64_t id, const char* data, size_t data_len); \
      void db_##IDX##_update( int iterator, uint64_t payer, const char* data, size_t data_len ); \
      void db_##IDX##_remove( int iterator ); \
      int db_##IDX##_find_secondary( uint64_t code, uint64_t scope, uint64_t table, const char* data, size_t data_len, uint64_t* primary ); \
      int db_##IDX##_find_primary( uint64_t code, uint64_t scope, uint64_t table, char* data, size_t data_len, uint64_t primary ); \
      int db_##IDX##_lowerbound( uint64_t code, uint64_t scope, uint64_t table, char* data, size_t data_len, uint64_t* primary ); \
      int db_##IDX##_upperbound( uint64_t code, uint64_t scope, uint64_t table, char* data, size_t data_len, uint64_t* primary ); \
      int db_##IDX##_end( uint64_t code, uint64_t scope, uint64_t table ); \
      int db_##IDX##_next( int iterator, uint64_t* primary  ); \
      int db_##IDX##_previous( int iterator, uint64_t* primary );

#define DB_API_METHOD_WRAPPERS_FLOAT_SECONDARY_DEF(IDX, TYPE)\
      int db_##IDX##_store( uint64_t scope, uint64_t table, uint64_t payer, uint64_t id, const TYPE& secondary ); \
      void db_##IDX##_update( int iterator, uint64_t payer, const TYPE& secondary ); \
      void db_##IDX##_remove( int iterator ); \
      int db_##IDX##_find_secondary( uint64_t code, uint64_t scope, uint64_t table, const TYPE& secondary, uint64_t& primary ); \
      int db_##IDX##_find_primary( uint64_t code, uint64_t scope, uint64_t table, TYPE& secondary, uint64_t primary ); \
      int db_##IDX##_lowerbound( uint64_t code, uint64_t scope, uint64_t table,  TYPE& secondary, uint64_t& primary ); \
      int db_##IDX##_upperbound( uint64_t code, uint64_t scope, uint64_t table,  TYPE& secondary, uint64_t& primary ); \
      int db_##IDX##_end( uint64_t code, uint64_t scope, uint64_t table ); \
      int db_##IDX##_next( int iterator, uint64_t& primary  ); \
      int db_##IDX##_previous( int iterator, uint64_t& primary );

DB_API_METHOD_WRAPPERS_SIMPLE_SECONDARY_DEF(idx64,  uint64_t)
DB_API_METHOD_WRAPPERS_SIMPLE_SECONDARY_DEF(idx128, eosio::chain::uint128_t)
DB_API_METHOD_WRAPPERS_ARRAY_SECONDARY_DEF(idx256, 2, eosio::chain::uint128_t)
//DB_API_METHOD_WRAPPERS_SIMPLE_SECONDARY_DEF(idx_double, uint64_t)
//DB_API_METHOD_WRAPPERS_FLOAT_SECONDARY_DEF(idx_double, float64_t)

struct eosapi {
   int (*wasm_call)(uint64_t _code, const char* _func, size_t _func_size, uint64_t* _args, size_t _args_size);
   uint64_t (*string_to_symbol)( uint8_t precision, const char* str );
   void (*eosio_delay)(int ms);
   uint32_t (*now)();
   void (*abort_)();
   void (*eosio_assert)(int condition, const char* str);
   //class crypto_api
   void (*assert_recover_key)( const char* data, size_t data_len,
                     const char* sig, size_t siglen,
                     const char* pub, size_t publen );
   mp_obj_t (*recover_key)(const char* data, size_t size, const char* sig, size_t siglen );
   void (*assert_sha256)(const char* data, size_t datalen, const char* hash, size_t hash_len);
   void (*assert_sha1)(const char* data, size_t datalen, const char* hash, size_t hash_len);
   void (*assert_sha512)(const char* data, size_t datalen, const char* hash, size_t hash_len);
   void (*assert_ripemd160)(const char* data, size_t datalen, const char* hash, size_t hash_len);
   mp_obj_t (*sha1)(const char* data, size_t datalen);
   mp_obj_t (*sha256)(const char* data, size_t datalen);
   mp_obj_t (*sha512)(const char* data, size_t datalen);
   mp_obj_t (*ripemd160)(const char* data, size_t datalen);

   uint64_t (*string_to_uint64_)(const char* str);
   mp_obj_t (*uint64_to_string_)(uint64_t n);

   #define Name uint64_t

   mp_obj_t (*pack_)(const char* str, int nsize);
   mp_obj_t (*unpack_)(const char* str, int nsize);

   //int get_account_balance_)(Name account, uint64_t& eos_balance, uint64_t& staked_balance, uint32_t& unstaking_balance, uint32_t& last_unstaking_time);
   mp_obj_t (*get_account_balance_)(Name account);
   uint64_t (*get_active_producers_)();
   mp_obj_t  (*sha256_)(const char* str, int nsize);

   int (*read_transaction)( char* data, size_t data_len );
   int (*transaction_size)();
   int (*expiration)();
   int (*tapos_block_num)();
   int (*tapos_block_prefix)();
   int (*get_action)( uint32_t type, uint32_t index, char* buffer, size_t buffer_size );


   void (*require_auth)(uint64_t account);
   void (*require_auth_ex)(uint64_t account, uint64_t permission);
   void (*require_write_lock)(uint64_t scope);
   void (*require_read_lock)(uint64_t account, uint64_t scope);
   int (*is_account)(uint64_t account);
   void (*require_recipient)(uint64_t account);


   int (*read_action)(char* memory, size_t size);
   int (*action_size)();
   uint64_t (*current_receiver)();
   uint64_t (*publication_time)();
   uint64_t (*current_sender)();

   int (*db_store_i64)( uint64_t scope, uint64_t table, uint64_t payer, uint64_t id, const char* buffer, size_t buffer_size );
   void (*db_update_i64)( int itr, uint64_t payer, const char* buffer, size_t buffer_size );
   void (*db_remove_i64)( int itr );
   int (*db_get_i64)( int itr, char* buffer, size_t buffer_size );
   int (*db_get_i64_ex)( int itr, uint64_t* primary, char* buffer, size_t buffer_size );
   int (*db_next_i64)( int itr, uint64_t* primary );
   int (*db_previous_i64)( int itr, uint64_t* primary );
   int (*db_find_i64)( uint64_t code, uint64_t scope, uint64_t table, uint64_t id );
   int (*db_lowerbound_i64)( uint64_t code, uint64_t scope, uint64_t table, uint64_t id );
   int (*db_upperbound_i64)( uint64_t code, uint64_t scope, uint64_t table, uint64_t id );
   int (*db_end_i64)( uint64_t code, uint64_t scope, uint64_t table );

   int (*db_idx64_store)( uint64_t scope, uint64_t table, uint64_t payer, uint64_t id, const char* secondary , size_t len);
   void (*db_idx64_update)( int iterator, uint64_t payer, const char* secondary , size_t len );
   void (*db_idx64_remove)( int iterator );
   int (*db_idx64_find_secondary)( uint64_t code, uint64_t scope, uint64_t table, const char* secondary , size_t len, uint64_t* primary );
   int (*db_idx64_find_primary)( uint64_t code, uint64_t scope, uint64_t table, char* secondary , size_t len, uint64_t primary );
   int (*db_idx64_lowerbound)( uint64_t code, uint64_t scope, uint64_t table, char* secondary , size_t len, uint64_t* primary );
   int (*db_idx64_upperbound)( uint64_t code, uint64_t scope, uint64_t table, char* secondary , size_t len, uint64_t* primary );
   int (*db_idx64_end)( uint64_t code, uint64_t scope, uint64_t table );
   int (*db_idx64_next)( int iterator, uint64_t* primary  );
   int (*db_idx64_previous)( int iterator, uint64_t* primary );

   int (*db_idx128_store)( uint64_t scope, uint64_t table, uint64_t payer, uint64_t id, const char* secondary , size_t len);
   void (*db_idx128_update)( int iterator, uint64_t payer, const char* secondary , size_t len );
   void (*db_idx128_remove)( int iterator );
   int (*db_idx128_find_secondary)( uint64_t code, uint64_t scope, uint64_t table, const char* secondary , size_t len, uint64_t* primary );
   int (*db_idx128_find_primary)( uint64_t code, uint64_t scope, uint64_t table, char* secondary , size_t len, uint64_t primary );
   int (*db_idx128_lowerbound)( uint64_t code, uint64_t scope, uint64_t table, char* secondary , size_t len, uint64_t* primary );
   int (*db_idx128_upperbound)( uint64_t code, uint64_t scope, uint64_t table, char* secondary , size_t len, uint64_t* primary );
   int (*db_idx128_end)( uint64_t code, uint64_t scope, uint64_t table );
   int (*db_idx128_next)( int iterator, uint64_t* primary  );
   int (*db_idx128_previous)( int iterator, uint64_t* primary );

   int (*db_idx256_store)( uint64_t scope, uint64_t table, uint64_t payer, uint64_t id, const char* secondary , size_t len);
   void (*db_idx256_update)( int iterator, uint64_t payer, const char* secondary , size_t len );
   void (*db_idx256_remove)( int iterator );
   int (*db_idx256_find_secondary)( uint64_t code, uint64_t scope, uint64_t table, const char* secondary , size_t len, uint64_t* primary );
   int (*db_idx256_find_primary)( uint64_t code, uint64_t scope, uint64_t table, char* secondary , size_t len, uint64_t primary );
   int (*db_idx256_lowerbound)( uint64_t code, uint64_t scope, uint64_t table, char* secondary , size_t len, uint64_t* primary );
   int (*db_idx256_upperbound)( uint64_t code, uint64_t scope, uint64_t table, char* secondary , size_t len, uint64_t* primary );
   int (*db_idx256_end)( uint64_t code, uint64_t scope, uint64_t table );
   int (*db_idx256_next)( int iterator, uint64_t* primary  );
   int (*db_idx256_previous)( int iterator, uint64_t* primary );

   int (*db_idx_double_store)( uint64_t scope, uint64_t table, uint64_t payer, uint64_t id, const char* secondary , size_t len);
   void (*db_idx_double_update)( int iterator, uint64_t payer, const char* secondary , size_t len );
   void (*db_idx_double_remove)( int iterator );
   int (*db_idx_double_find_secondary)( uint64_t code, uint64_t scope, uint64_t table, const char* secondary , size_t len, uint64_t* primary );
   int (*db_idx_double_find_primary)( uint64_t code, uint64_t scope, uint64_t table, char* secondary , size_t len, uint64_t primary );
   int (*db_idx_double_lowerbound)( uint64_t code, uint64_t scope, uint64_t table, char* secondary , size_t len, uint64_t* primary );
   int (*db_idx_double_upperbound)( uint64_t code, uint64_t scope, uint64_t table, char* secondary , size_t len, uint64_t* primary );
   int (*db_idx_double_end)( uint64_t code, uint64_t scope, uint64_t table );
   int (*db_idx_double_next)( int iterator, uint64_t* primary  );
   int (*db_idx_double_previous)( int iterator, uint64_t* primary );

   void (*send_inline)( struct mp_action* mp_act );
   void (*send_context_free_inline)(struct mp_action* mp_act);
   void (*send_deferred)( uint128_t sender_id, uint64_t payer, struct mp_transaction* mp_trx );
   void (*cancel_deferred)( uint128_t  val );

   int (*split_path)(const char* str_path, char *path1, size_t path1_size, char *path2, size_t path2_size);

   uint64_t (*get_action_account)();
};



typedef void (*fn_printer)(const char * str, size_t len);

struct mpapi {
   int init;
   void *handle;
   struct eosapi *_eosapi;
   mp_obj_t (*mp_obj_new_str)(const char* data, size_t len);
   mp_obj_t (*mp_obj_new_bytes)(const byte* data, size_t len);

   mp_obj_t (*micropy_load_from_py)(const char *mod_name, const char *data, size_t len);
   mp_obj_t (*micropy_load_from_mpy)(const char *mod_name, const char *data, size_t len);
   mp_obj_t (*micropy_call_0)(mp_obj_t module_obj, const char *func);
   mp_obj_t (*micropy_call_2)(mp_obj_t module_obj, const char *func, uint64_t code, uint64_t type);
   mp_obj_t (*micropy_call_3)(mp_obj_t module_obj, const char *func, uint64_t receiver, uint64_t code, uint64_t type);

   void* (*execute_from_str)(const char *str);

   //defined in vm.c
   void (*execution_start)();
   void (*execution_end)();
   void (*set_max_execution_time)(int time);
   uint64_t (*get_execution_time)();

   //main_eos.c
   int (*main_micropython)(int argc, char **argv);
   mp_obj_t (*mp_call_function_0)(mp_obj_t fun);
   mp_obj_t (*mp_compile)(mp_parse_tree_t *parse_tree, qstr source_file, uint emit_opt, bool is_repl);
   int (*compile_and_save_to_buffer)(const char* src_name, const char *src_buffer, size_t src_size, char* buffer, size_t size);
   void (*set_debug_mode)(int mode);
   void (*set_printer)(fn_printer _printer);

};


typedef void (*fn_mp_register_eosapi)(struct eosapi * _api);
typedef void (*fn_mp_obtain_mpapi)(struct mpapi * _api);
typedef int (*fn_main_micropython)(int argc, char **argv);

int wasm_call_(uint64_t _code, const char* _func, size_t _func_size, uint64_t* _args, size_t _args_size);
void eosio_delay(int ms);

int micropython_on_apply(uint64_t receiver, uint64_t account, uint64_t act, char** err);

int split_path(const char* str_path, char *path1, size_t path1_size, char *path2, size_t path2_size);

#ifdef __cplusplus
   }
#endif

#ifdef __cplusplus
   mpapi& get_mpapi();
#endif


#endif /* MPEOSLIB__HPP_ */
