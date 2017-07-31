/*
 * Copyright (c) 2017, Respective Authors.
 *
 * The MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#pragma once
#include <eos/chain/types.hpp>
#include <eos/chain/authority.hpp>

#include "multi_index_includes.hpp"

namespace eos { namespace chain {

   class account_object : public chainbase::object<account_object_type, account_object> {
      OBJECT_CTOR(account_object,(code)(abi))

      id_type             id;
      AccountName         name;
      uint8_t             vm_type      = 0;
      uint8_t             vm_version   = 0;
      fc::sha256          code_version;
      Time                creation_date;
      shared_vector<char> code;
      shared_vector<char> abi;

      void set_abi( const eos::types::Abi& _abi ) {
         abi.resize( fc::raw::pack_size( _abi ) );
         fc::datastream<char*> ds( abi.data(), abi.size() );
         fc::raw::pack( ds, _abi );
      }
   };
   using account_id_type = account_object::id_type;

   struct by_name;
   using account_index = chainbase::shared_multi_index_container<
      account_object,
      indexed_by<
         ordered_unique<tag<by_id>, member<account_object, account_object::id_type, &account_object::id>>,
         ordered_unique<tag<by_name>, member<account_object, AccountName, &account_object::name>>
      >
   >;


   class auth_cache_object : public chainbase::object<auth_cache_object_type, auth_cache_object> {
      OBJECT_CTOR(auth_cache_object)

      id_type id;
      AccountName    scope;
      AccountName    code;
      AccountName    auth;
      PermissionName permission;
   };

   struct by_scope_code_auth_permission;
   using auth_cache_index = chainbase::shared_multi_index_container<
      auth_cache_object,
      indexed_by<
         ordered_unique<tag<by_id>, member<auth_cache_object, auth_cache_object::id_type, &auth_cache_object::id>>,
         ordered_unique<tag<by_scope_code_auth_permission>, 
            composite_key< auth_cache_object,
                member<auth_cache_object, AccountName, &auth_cache_object::scope>,
                member<auth_cache_object, AccountName, &auth_cache_object::code>,
                member<auth_cache_object, AccountName, &auth_cache_object::auth>,
                member<auth_cache_object, PermissionName, &auth_cache_object::permission>
            >
         >
       >
    >;


} } // eos::chain

CHAINBASE_SET_INDEX_TYPE(eos::chain::account_object, eos::chain::account_index)
CHAINBASE_SET_INDEX_TYPE(eos::chain::auth_cache_object, eos::chain::auth_cache_index)

FC_REFLECT(chainbase::oid<eos::chain::account_object>, (_id))

FC_REFLECT(eos::chain::account_object, (id)(name)(vm_type)(vm_version)(code_version)(code)(creation_date))
