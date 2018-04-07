/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */

#include <eosiolib/currency.hpp>
#include <eosiolib/print.h>
extern "C" {
    /// The apply method implements the dispatch of events to this contract
    void apply( uint64_t receiver, uint64_t code, uint64_t action ) {
       prints("hello,world");
       eosio::currency(receiver).apply( code, action ); 
    }
}
