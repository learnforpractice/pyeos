#include "eosio.system.hpp"
#include <eosiolib/dispatcher.hpp>

#include "producer_pay.cpp"
#include "delegate_bandwidth.cpp"
#include "voting.cpp"
#include "exchange_state.cpp"


namespace eosiosystem {

   system_contract::system_contract( account_name s )
   :native(s),
    _voters(_self,_self),
    _producers(_self,_self),
    _global(_self,_self),
    _rammarket(_self,_self),
    _jitbid(_self,_self),
    _boost(_self,_self)
    {
      //print( "construct system\n" );
      _gstate = _global.exists() ? _global.get() : get_default_parameters();
      if (!_jitbid.exists()) {
         jit_bid bid;
         bid.high_bidder = 0;
         bid.high_bid = 0;
         bid.last_bid_time = 0;
         bid.start_bid_time = current_time();
         bid.jit_remains = 100; //TODO: max jit resources, should be configurable
         _jitbid.set(bid, _self);
         _boost.emplace( _self, [&]( auto& b ) {
               b.account = N(eosio);
               b.expiration = (uint64_t)-1;
         });
         _boost.emplace( _self, [&]( auto& b ) {
               b.account = N(eosio.token);
               b.expiration = (uint64_t)-1;
         });
      }

      auto itr = _rammarket.find(S(4,RAMCORE));

      if( itr == _rammarket.end() ) {
         auto system_token_supply   = eosio::token(N(eosio.token)).get_supply(eosio::symbol_type(system_token_symbol).name()).amount;
         if( system_token_supply > 0 ) {
            itr = _rammarket.emplace( _self, [&]( auto& m ) {
               m.supply.amount = 100000000000000ll;
               m.supply.symbol = S(4,RAMCORE);
               m.base.balance.amount = int64_t(_gstate.free_ram());
               m.base.balance.symbol = S(0,RAM);
               m.quote.balance.amount = system_token_supply / 1000;
               m.quote.balance.symbol = CORE_SYMBOL;
            });
         }
      } else {
         //print( "ram market already created" );
      }
   }

   eosio_global_state system_contract::get_default_parameters() {
      eosio_global_state dp;
      get_blockchain_parameters(dp);
      return dp;
   }


   system_contract::~system_contract() {
      //print( "destruct system\n" );
      _global.set( _gstate, _self );
      //eosio_exit(0);
   }

   void system_contract::setram( uint64_t max_ram_size ) {
      require_auth( _self );

      eosio_assert( _gstate.max_ram_size < max_ram_size, "ram may only be increased" ); /// decreasing ram might result market maker issues
      eosio_assert( max_ram_size < 1024ll*1024*1024*1024*1024, "ram size is unrealistic" );
      eosio_assert( max_ram_size > _gstate.total_ram_bytes_reserved, "attempt to set max below reserved" );

      auto delta = int64_t(max_ram_size) - int64_t(_gstate.max_ram_size);
      auto itr = _rammarket.find(S(4,RAMCORE));

      /**
       *  Increase or decrease the amount of ram for sale based upon the change in max
       *  ram size.
       */
      _rammarket.modify( itr, 0, [&]( auto& m ) {
         m.base.balance.amount += delta;
      });

      _gstate.max_ram_size = max_ram_size;
      _global.set( _gstate, _self );
   }

   void system_contract::setparams( const eosio::blockchain_parameters& params ) {
      require_auth( N(eosio) );
      (eosio::blockchain_parameters&)(_gstate) = params;
      eosio_assert( 3 <= _gstate.max_authority_depth, "max_authority_depth should be at least 3" );
      set_blockchain_parameters( params );
   }

   void system_contract::setpriv( account_name account, uint8_t ispriv ) {
      require_auth( _self );
      set_privileged( account, ispriv );
   }

   void system_contract::rmvproducer( account_name producer ) {
      require_auth( _self );
      auto prod = _producers.find( producer );
      eosio_assert( prod != _producers.end(), "producer not found" );
      _producers.modify( prod, 0, [&](auto& p) {
            p.deactivate();
         });
   }

   void system_contract::bidname( account_name bidder, account_name newname, asset bid ) {
      require_auth( bidder );
      eosio_assert( eosio::name_suffix(newname) == newname, "you can only bid on top-level suffix" );
      eosio_assert( newname != 0, "the empty name is not a valid account name to bid on" );
      eosio_assert( (newname & 0xFull) == 0, "13 character names are not valid account names to bid on" );
      eosio_assert( (newname & 0x1F0ull) == 0, "accounts with 12 character names and no dots can be created without bidding required" );
      eosio_assert( !is_account( newname ), "account already exists" );
      eosio_assert( bid.symbol == asset().symbol, "asset must be system token" );
      eosio_assert( bid.amount > 0, "insufficient bid" );

      INLINE_ACTION_SENDER(eosio::token, transfer)( N(eosio.token), {bidder,N(active)},
                                                    { bidder, N(eosio.names), bid, std::string("bid name ")+(name{newname}).to_string()  } );

      name_bid_table bids(_self,_self);
      print( name{bidder}, " bid ", bid, " on ", name{newname}, "\n" );
      auto current = bids.find( newname );
      if( current == bids.end() ) {
         bids.emplace( bidder, [&]( auto& b ) {
            b.newname = newname;
            b.high_bidder = bidder;
            b.high_bid = bid.amount;
            b.last_bid_time = current_time();
         });
      } else {
         eosio_assert( current->high_bid > 0, "this auction has already closed" );
         eosio_assert( bid.amount - current->high_bid > (current->high_bid / 10), "must increase bid by 10%" );
         eosio_assert( current->high_bidder != bidder, "account is already highest bidder" );

         INLINE_ACTION_SENDER(eosio::token, transfer)( N(eosio.token), {N(eosio.names),N(active)},
                                                       { N(eosio.names), current->high_bidder, asset(current->high_bid),
                                                       std::string("refund bid on name ")+(name{newname}).to_string()  } );

         bids.modify( current, bidder, [&]( auto& b ) {
            b.high_bidder = bidder;
            b.high_bid = bid.amount;
            b.last_bid_time = current_time();
         });
      }
   }

   void system_contract::bidjit( account_name bidder, asset bid ) {
       require_auth( bidder );
       eosio_assert( bid.symbol == asset().symbol, "asset must be system token" );
       eosio_assert( bid.amount > 0, "insufficient bid" );

       jit_bid current = _jitbid.get();

//       eosio_assert( current.high_bid > 0, "this auction has already closed" );
       eosio_assert( bid.amount - current.high_bid > (current.high_bid / 10), "must increase bid by 10%" );
       eosio_assert( current.high_bidder != bidder, "account is already highest bidder" );
       eosio_assert( current.jit_remains > 0, "no jit resources left!" );


       INLINE_ACTION_SENDER(eosio::token, transfer)( N(eosio.token), {bidder,N(active)},
                                                     { bidder, N(eosio.jit), bid, std::string("bid jit ")+(name{bidder}).to_string()  } );
       if (current.high_bidder) {
          INLINE_ACTION_SENDER(eosio::token, transfer)( N(eosio.token), {N(eosio.jit),N(active)},
                                                        { N(eosio.jit), current.high_bidder, asset(current.high_bid),
                                                        std::string("refund bid to ")+(name{bidder}).to_string()  } );
       }
       current.high_bidder = bidder;
       current.high_bid = bid.amount;
       current.last_bid_time = current_time();
       _jitbid.set(current, _self);
    }

   void system_contract::boost(account_name account) {
      require_auth( N(eosio) );
      eosio_assert(is_account(account), "account does not exist");
       eosio_assert(_boost.find(account) == _boost.end(), "account already accelerated");
      _boost.emplace( N(eosio), [&]( auto& p ) {
            p.account = account;
            p.expiration = uint64_t(-1); //no expiration
      });
      jit_bid bid = _jitbid.get();
      bid.jit_remains -= 1;
      _jitbid.set(bid, 0);
   }

   void system_contract::cancelboost(account_name account) {
      require_auth( N(eosio) );
      eosio_assert(is_account(account), "account does not exist");
      auto itr = _boost.find(account);
      eosio_assert( itr != _boost.end(), "account not in list" );
      _boost.erase(itr);
      jit_bid bid = _jitbid.get();
      bid.jit_remains += 1;
      _jitbid.set(bid, 0);
   }

   /**
    *  Called after a new account is created. This code enforces resource-limits rules
    *  for new accounts as well as new account naming conventions.
    *
    *  Account names containing '.' symbols must have a suffix equal to the name of the creator.
    *  This allows users who buy a premium name (shorter than 12 characters with no dots) to be the only ones
    *  who can create accounts with the creator's name as a suffix.
    *
    */
   void native::newaccount( account_name     creator,
                            account_name     newact
                            /*  no need to parse authorities
                            const authority& owner,
                            const authority& active*/ ) {

      if( creator != _self ) {
         auto tmp = newact >> 4;
         bool has_dot = false;

         for( uint32_t i = 0; i < 12; ++i ) {
           has_dot |= !(tmp & 0x1f);
           tmp >>= 5;
         }
         if( has_dot ) { // or is less than 12 characters
            auto suffix = eosio::name_suffix(newact);
            if( suffix == newact ) {
               name_bid_table bids(_self,_self);
               auto current = bids.find( newact );
               eosio_assert( current != bids.end(), "no active bid for name" );
               eosio_assert( current->high_bidder == creator, "only highest bidder can claim" );
               eosio_assert( current->high_bid < 0, "auction for name is not closed yet" );
               bids.erase( current );
            } else {
               eosio_assert( creator == suffix, "only suffix may create this account" );
            }
         }
      }

      user_resources_table  userres( _self, newact);

      userres.emplace( newact, [&]( auto& res ) {
        res.owner = newact;
      });

      set_resource_limits( newact, 0, 0, 0 );
   }

} /// eosio.system


EOSIO_ABI( eosiosystem::system_contract,
     // native.hpp (newaccount definition is actually in eosio.system.cpp)
     (newaccount)(updateauth)(deleteauth)(linkauth)(unlinkauth)(canceldelay)(onerror)
     // eosio.system.cpp
     (setram)(setparams)(setpriv)(rmvproducer)(bidname)(boost)(cancelboost)(bidjit)
     // delegate_bandwidth.cpp
     (buyrambytes)(buyram)(sellram)(delegatebw)(undelegatebw)(refund)
     // voting.cpp
     (regproducer)(unregprod)(voteproducer)(regproxy)
     // producer_pay.cpp
     (onblock)(claimrewards)
)
