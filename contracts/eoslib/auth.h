/**
 *  @file auth.h
 *  @brief Defines C API for interfacing with authority caching
 */
#pragma once
#include <eoslib/types.hpp>

extern "C" {

/**
 * @defgroup authcache Authority Cache
 * @brief APIs for saving, using, and clearing authorization
 * @ingroup contractdev
 *
 * There are cases where one contract needs to generate a message in the name of another
 * contract. A contract can cache the authority on the current message and then use that
 * authority while generating a synchronous message in the future.
 *
 * These methods require the specified scope. An authority cached in one scope cannot be
 * loaded in another.  
 * 
 * @{
 */
void cache_auth( AccountName scope, AccountName account, PermissionName perm );
void load_auth( AccountName scope, AccountName account, PermissionName perm );
void clear_auth( AccountName scope, AccountName account, PermissionName perm );
///@} 

}
