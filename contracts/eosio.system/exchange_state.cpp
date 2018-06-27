#include <exchange/exchange_state.hpp>

extern "C" uint64_t wasm_call(const char*func, uint64_t* args , int argc);

namespace std {
   double _pow(double base, double power) {
      uint64_t args[2];
      args[0] = *(uint64_t*)&base;
      args[1] = *(uint64_t*)&power;
      uint64_t ret = wasm_call("mypow", args, 2);
      return *(double*)&ret;
   }
}

namespace eosiosystem {
   asset exchange_state::convert_to_exchange( connector& c, asset in ) {
      real_type R(supply.amount);
      real_type C(c.balance.amount+in.amount);
      real_type F(c.weight/1000.0);
      real_type T(in.amount);
      real_type ONE(1.0);

      real_type E = -R * (ONE - std::_pow( ONE + T / C, F) );
      //print( "E: ", E, "\n");
      int64_t issued = int64_t(E);

      supply.amount += issued;
      c.balance.amount += in.amount;

      return asset( issued, supply.symbol );
   }

   asset exchange_state::convert_from_exchange( connector& c, asset in ) {
      eosio_assert( in.symbol== supply.symbol, "unexpected asset symbol input" );

      real_type R(supply.amount - in.amount);
      real_type C(c.balance.amount);
      real_type F(Double(1000.0)/Double(c.weight));
      real_type E(in.amount);
      real_type ONE(1.0);


     // potentially more accurate: 
     // The functions std::expm1 and std::log1p are useful for financial calculations, for example, 
     // when calculating small daily interest rates: (1+x)n
     // -1 can be expressed as std::expm1(n * std::log1p(x)). 
     // real_type T = C * std::expm1( F * std::log1p(E/R) );
      
      real_type T = C * (std::_pow( ONE + E/R, F) - ONE);
      //print( "T: ", T, "\n");
      int64_t out = int64_t(T);

      supply.amount -= in.amount;
      c.balance.amount -= out;

      return asset( out, c.balance.symbol );
   }

   asset exchange_state::convert( asset from, symbol_type to ) {
      auto sell_symbol  = from.symbol;
      auto ex_symbol    = supply.symbol;
      auto base_symbol  = base.balance.symbol;
      auto quote_symbol = quote.balance.symbol;

      //print( "From: ", from, " TO ", asset( 0,to), "\n" );
      //print( "base: ", base_symbol, "\n" );
      //print( "quote: ", quote_symbol, "\n" );
      //print( "ex: ", supply.symbol, "\n" );

      if( sell_symbol != ex_symbol ) {
         if( sell_symbol == base_symbol ) {
            from = convert_to_exchange( base, from );
         } else if( sell_symbol == quote_symbol ) {
            from = convert_to_exchange( quote, from );
         } else { 
            eosio_assert( false, "invalid sell" );
         }
      } else {
         if( to == base_symbol ) {
            from = convert_from_exchange( base, from ); 
         } else if( to == quote_symbol ) {
            from = convert_from_exchange( quote, from ); 
         } else {
            eosio_assert( false, "invalid conversion" );
         }
      }

      if( to != from.symbol )
         return convert( from, to );

      return from;
   }



} /// namespace eosiosystem
