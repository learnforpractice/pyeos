#include <eosio/chain/options.hpp>

namespace eosio { namespace chain {

options::options(): _options(appbase::app().get_variables_map()) {
   if( _options.count( "wasm-runtime" )) {
      wasm_runtime = _options.at( "wasm-runtime" ).as<vm_type>();
   }
}

int options::get_wasm_runtime_type() {
   return (int)wasm_runtime;
}

options::~options() {

}


}
}
