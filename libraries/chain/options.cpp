#include <eosio/chain/options.hpp>

namespace eosio { namespace chain {

options::options(): _options(appbase::app().get_variables_map()) {
   if( _options.count( "wasm-runtime" )) {
      wasm_runtime = _options.at( "wasm-runtime" ).as<vm_type>();
   }

   if( _options.count( "contracts-console" )) {
      contracts_console = _options.at( "contracts-console" ).as<bool>();
   }
}

bool options::is_contracts_console_enabled() {
   return contracts_console;
}


int options::get_wasm_runtime_type() {
   return (int)wasm_runtime;
}

options::~options() {

}


}
}
