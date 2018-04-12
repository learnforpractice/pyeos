/*
 * db_handling.hpp
 *
 *  Created on: Nov 12, 2017
 *      Author: newworld
 */

#ifndef PROGRAMS_PYEOS_UTIL_DB_HANDLING_HPP_
#define PROGRAMS_PYEOS_UTIL_DB_HANDLING_HPP_

#include <boost/algorithm/cxx11/all_of.hpp>
#include <boost/range/algorithm/find_if.hpp>

#include <appbase/application.hpp>
#include <eosio/chain_plugin/chain_plugin.hpp>


namespace chainbase { class database; }

using namespace eosio;
using namespace eosio::chain;

namespace python {
   class db_handling {
   public:
      db_handling(chainbase::database& db);

      template <typename ObjectType>
      int32_t store_record( name scope, name code, name table, typename ObjectType::key_type* keys, char* value, uint32_t valuelen ) {

         auto tuple = find_tuple<ObjectType>::get(scope, code, table, keys);
         const auto* obj = db.find<ObjectType, by_scope_primary>(tuple);

         if( obj ) {
            //wlog( "modify" );
            mutable_db.modify( *obj, [&]( auto& o ) {
               o.value.assign(value, valuelen);
            });
            return 0;
         } else {
            //wlog( "new" );
            mutable_db.create<ObjectType>( [&](auto& o) {
               o.scope = scope;
               o.code  = code;
               o.table = table;
               key_helper<ObjectType>::set(o, keys);
               o.value.insert( 0, value, valuelen );
            });
            return 1;
         }
      }

      template <typename ObjectType>
      int32_t update_record( name scope, name code, name table, typename ObjectType::key_type *keys, char* value, uint32_t valuelen ) {

         auto tuple = find_tuple<ObjectType>::get(scope, code, table, keys);
         const auto* obj = db.find<ObjectType, by_scope_primary>(tuple);

         if( !obj ) {
            return 0;
         }

         mutable_db.modify( *obj, [&]( auto& o ) {
            if( valuelen > o.value.size() ) {
               o.value.resize(valuelen);
            }
            memcpy(o.value.data(), value, valuelen);
         });

         return 1;
      }

      template <typename ObjectType>
      int32_t remove_record( name scope, name code, name table, typename ObjectType::key_type* keys, char* value, uint32_t valuelen ) {

         auto tuple = find_tuple<ObjectType>::get(scope, code, table, keys);
         const auto* obj = db.find<ObjectType, by_scope_primary>(tuple);
         if( obj ) {
            mutable_db.remove( *obj );
            return 1;
         }
         return 0;
      }

      template <typename IndexType, typename Scope>
      int32_t load_record( name scope, name code, name table, typename IndexType::value_type::key_type* keys, char* value, uint32_t valuelen ) {

         const auto& idx = db.get_index<IndexType, Scope>();
         auto tuple = load_record_tuple<typename IndexType::value_type, Scope>::get(scope, code, table, keys);
         auto itr = idx.lower_bound(tuple);

         if( itr == idx.end() ||
             itr->scope != scope ||
             itr->code  != code  ||
             itr->table != table ||
             !load_record_compare<typename IndexType::value_type, Scope>::compare(*itr, keys)) return -1;

          key_helper<typename IndexType::value_type>::set(keys, *itr);

          auto copylen =  std::min<size_t>(itr->value.size(),valuelen);
          if( copylen ) {
             itr->value.copy(value, copylen);
          }
          return copylen;
      }

      template <typename IndexType, typename Scope>
      int32_t front_record( name scope, name code, name table, typename IndexType::value_type::key_type* keys, char* value, uint32_t valuelen ) {

         const auto& idx = db.get_index<IndexType, Scope>();
         auto tuple = front_record_tuple<typename IndexType::value_type>::get(scope, code, table);

         auto itr = idx.lower_bound( tuple );
         if( itr == idx.end() ||
             itr->scope != scope ||
             itr->code  != code  ||
             itr->table != table ) return -1;

         key_helper<typename IndexType::value_type>::set(keys, *itr);

         auto copylen =  std::min<size_t>(itr->value.size(),valuelen);
         if( copylen ) {
            itr->value.copy(value, copylen);
         }
         return copylen;
      }

      template <typename IndexType, typename Scope>
      int32_t back_record( name scope, name code, name table, typename IndexType::value_type::key_type* keys, char* value, uint32_t valuelen ) {

         const auto& idx = db.get_index<IndexType, Scope>();
         auto tuple = boost::make_tuple( account_name(scope), account_name(code), account_name(uint64_t(table)+1) );
         auto itr = idx.lower_bound(tuple);

         if( std::distance(idx.begin(), itr) == 0 ) return -1;

         --itr;

         if( itr->scope != scope ||
             itr->code  != code  ||
             itr->table != table ) return -1;

         key_helper<typename IndexType::value_type>::set(keys, *itr);

         auto copylen =  std::min<size_t>(itr->value.size(),valuelen);
         if( copylen ) {
            itr->value.copy(value, copylen);
         }
         return copylen;
      }

      template <typename IndexType, typename Scope>
      int32_t next_record( name scope, name code, name table, typename IndexType::value_type::key_type* keys, char* value, uint32_t valuelen ) {

         const auto& pidx = db.get_index<IndexType, by_scope_primary>();

         auto tuple = next_record_tuple<typename IndexType::value_type>::get(scope, code, table, keys);
         auto pitr = pidx.find(tuple);

         if(pitr == pidx.end())
           return -1;

         const auto& fidx = db.get_index<IndexType>();
         auto itr = fidx.indicies().template project<Scope>(pitr);

         const auto& idx = db.get_index<IndexType, Scope>();

         if( itr == idx.end() ||
             itr->scope != scope ||
             itr->code  != code  ||
             itr->table != table ||
             !key_helper<typename IndexType::value_type>::compare(*itr, keys) ) {
           return -1;
         }

         ++itr;

         if( itr == idx.end() ||
             itr->scope != scope ||
             itr->code  != code  ||
             itr->table != table ) {
           return -1;
         }

         key_helper<typename IndexType::value_type>::set(keys, *itr);

         auto copylen =  std::min<size_t>(itr->value.size(),valuelen);
         if( copylen ) {
            itr->value.copy(value, copylen);
         }
         return copylen;
      }

      template <typename IndexType, typename Scope>
      int32_t previous_record( name scope, name code, name table, typename IndexType::value_type::key_type* keys, char* value, uint32_t valuelen ) {

         const auto& pidx = db.get_index<IndexType, by_scope_primary>();

         auto tuple = next_record_tuple<typename IndexType::value_type>::get(scope, code, table, keys);
         auto pitr = pidx.find(tuple);

         if(pitr == pidx.end())
           return -1;

         const auto& fidx = db.get_index<IndexType>();
         auto itr = fidx.indicies().template project<Scope>(pitr);

         const auto& idx = db.get_index<IndexType, Scope>();

         if( itr == idx.end() ||
             itr == idx.begin() ||
             itr->scope != scope ||
             itr->code  != code  ||
             itr->table != table ||
             !key_helper<typename IndexType::value_type>::compare(*itr, keys) ) return -1;

         --itr;

         if( itr->scope != scope ||
             itr->code  != code  ||
             itr->table != table ) return -1;

         key_helper<typename IndexType::value_type>::set(keys, *itr);

         auto copylen =  std::min<size_t>(itr->value.size(),valuelen);
         if( copylen ) {
            itr->value.copy(value, copylen);
         }
         return copylen;
      }

      template <typename IndexType, typename Scope>
      int32_t lower_bound_record( name scope, name code, name table, typename IndexType::value_type::key_type* keys, char* value, uint32_t valuelen ) {

         const auto& idx = db.get_index<IndexType, Scope>();
         auto tuple = lower_bound_tuple<typename IndexType::value_type, Scope>::get(scope, code, table, keys);
         auto itr = idx.lower_bound(tuple);

         if( itr == idx.end() ||
             itr->scope != scope ||
             itr->code  != code  ||
             itr->table != table) return -1;

         key_helper<typename IndexType::value_type>::set(keys, *itr);

         auto copylen =  std::min<size_t>(itr->value.size(),valuelen);
         if( copylen ) {
            itr->value.copy(value, copylen);
         }
         return copylen;
      }

      template <typename IndexType, typename Scope>
      int32_t upper_bound_record( name scope, name code, name table, typename IndexType::value_type::key_type* keys, char* value, uint32_t valuelen ) {

         const auto& idx = db.get_index<IndexType, Scope>();
         auto tuple = upper_bound_tuple<typename IndexType::value_type, Scope>::get(scope, code, table, keys);
         auto itr = idx.upper_bound(tuple);

         if( itr == idx.end() ||
             itr->scope != scope ||
             itr->code  != code  ||
             itr->table != table ) return -1;

         key_helper<typename IndexType::value_type>::set(keys, *itr);

         auto copylen =  std::min<size_t>(itr->value.size(),valuelen);
         if( copylen ) {
            itr->value.copy(value, copylen);
         }
         return copylen;
      }

   private:
      const chainbase::database&   db;  ///< database where state is stored
      chainbase::database& mutable_db;
   };
}


#endif /* PROGRAMS_PYEOS_UTIL_DB_HANDLING_HPP_ */
