/*
 * db_api.h
 *
 *  Created on: Jul 15, 2018
 *      Author: newworld
 */

#ifndef LIBRARIES_DB_API_INCLUDE_EOSIO_CHAIN_DB_API_H_
#define LIBRARIES_DB_API_INCLUDE_EOSIO_CHAIN_DB_API_H_

#ifdef __cplusplus
extern "C" {
#endif

void db_api_remove_i64(int itr);

int db_api_get_i64( int itr, char* buffer, size_t buffer_size );
int32_t db_api_get_i64_ex( int iterator, uint64_t* primary, char* buffer, size_t buffer_size );
const char* db_api_get_i64_exex( int itr, size_t* buffer_size );

int db_api_next_i64( int itr, uint64_t* primary );
int db_api_previous_i64( int itr, uint64_t* primary );
int db_api_find_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id );
int db_api_lowerbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id );
int db_api_upperbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id );
int db_api_end_i64( uint64_t code, uint64_t scope, uint64_t table );

#ifdef __cplusplus
}
#endif

#endif /* LIBRARIES_DB_API_INCLUDE_EOSIO_CHAIN_DB_API_H_ */
