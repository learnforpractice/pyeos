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

#include "py/objlist.h"
#include "py/objstringio.h"
#include "py/runtime.h"
#include "py/stream.h"


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
mp_obj_t sha1(const char* data, size_t datalen);
mp_obj_t sha256(const char* data, size_t datalen);
mp_obj_t sha512(const char* data, size_t datalen);
mp_obj_t ripemd160(const char* data, size_t datalen);



uint64_t string_to_uint64_(const char* str);
mp_obj_t uint64_to_string_(uint64_t n);

#define Name uint64_t

mp_obj_t pack_(const char* str, int nsize);
mp_obj_t unpack_(const char* str, int nsize);

//int get_account_balance_(Name account, uint64_t& eos_balance, uint64_t& staked_balance, uint32_t& unstaking_balance, uint32_t& last_unstaking_time);
mp_obj_t get_account_balance_(Name account);
uint64_t get_active_producers_();
mp_obj_t  sha256_(const char* str, int nsize);

int read_transaction( char* data, size_t data_len );
int transaction_size();
int expiration();
int tapos_block_num();
int tapos_block_prefix();
int get_action( uint32_t type, uint32_t index, char* buffer, size_t buffer_size );


void require_auth(uint64_t account);
void require_auth_ex(uint64_t account, uint64_t permission);
void require_write_lock(uint64_t scope);
void require_read_lock(uint64_t account, uint64_t scope);
int is_account(uint64_t account);
void require_recipient(uint64_t account);


int read_action(char* memory, size_t size);
int action_size();
uint64_t current_receiver();
uint64_t publication_time();
uint64_t current_sender();



int db_store_i64( uint64_t scope, uint64_t table, uint64_t payer, uint64_t id, const char* buffer, size_t buffer_size );
void db_update_i64( int itr, uint64_t payer, const char* buffer, size_t buffer_size );
void db_remove_i64( int itr );
int db_get_i64( int itr, char* buffer, size_t buffer_size );
int db_next_i64( int itr, uint64_t* primary );
int db_previous_i64( int itr, uint64_t* primary );
int db_find_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id );
int db_lowerbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id );
int db_upperbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id );
int db_end_i64( uint64_t code, uint64_t scope, uint64_t table );

uint64_t get_action_account();

uint64_t string_to_symbol( uint8_t precision, const char* str );

struct eosapi {
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
   int (*db_next_i64)( int itr, uint64_t* primary );
   int (*db_previous_i64)( int itr, uint64_t* primary );
   int (*db_find_i64)( uint64_t code, uint64_t scope, uint64_t table, uint64_t id );
   int (*db_lowerbound_i64)( uint64_t code, uint64_t scope, uint64_t table, uint64_t id );
   int (*db_upperbound_i64)( uint64_t code, uint64_t scope, uint64_t table, uint64_t id );
   int (*db_end_i64)( uint64_t code, uint64_t scope, uint64_t table );
};

#ifdef __cplusplus
   }
#endif


#endif /* MPEOSLIB__HPP_ */
