#include "db_handling.hpp"
#include <chainbase/chainbase.hpp>

#include <eos/utilities/parallel_markers.hpp>

#include <fc/bitutil.hpp>

using namespace eosio::chain;

namespace python {
   db_handling::db_handling(chainbase::database& db) : db(db), mutable_db(db){
   }
}

