/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */

#include <stdint.h>
#include <wchar.h>


#ifndef TYPES_H
#define TYPES_H


#ifdef __cplusplus
extern "C" {
#endif

#ifndef __cplusplus
#ifndef bool
typedef char bool;
#endif
#endif

/**
 *  @defgroup types Builtin Types
 *  @ingroup contractdev
 *  @brief Specifies typedefs and aliases
 *
 *  @{
 */

typedef uint64_t account_name;
typedef uint64_t permission_name;
typedef uint64_t table_name;


typedef uint32_t _time;

typedef uint64_t scope_name;
typedef uint64_t action_name;

typedef uint16_t weight_type;

#ifndef __WASM
   typedef __int128 int128_t ;
   typedef unsigned __int128 uint128_t;
#endif


/* macro to align/overalign a type to ensure calls to intrinsics with pointers/references are properly aligned */
#define ALIGNED(X) __attribute__ ((aligned (16))) X

struct public_key {
   char data[34];
};

struct signature {
   uint8_t data[66];
};

#ifdef __WASM
struct ALIGNED(checksum256) {
   uint8_t hash[32];
};

struct ALIGNED(checksum160) {
   uint8_t hash[20];
};

struct ALIGNED(checksum512) {
   uint8_t hash[64];
};
#else
struct checksum256 {
   uint8_t hash[32];
};

struct checksum160 {
   uint8_t hash[20];
};

struct checksum512 {
   uint8_t hash[64];
};

#endif

typedef struct checksum256 transaction_id_type;
typedef struct checksum256 block_id_type;

struct account_permission {
   account_name account;
   permission_name permission;
};

#ifdef __cplusplus
} /// extern "C"
#endif
/// @}

#endif
