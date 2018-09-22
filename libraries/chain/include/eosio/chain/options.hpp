#pragma once
#include <appbase/application.hpp>
#include <boost/program_options.hpp>
#include <eosio/chain/wasm_interface.hpp>

namespace bpo = boost::program_options;
using bpo::options_description;
using bpo::variables_map;


namespace eosio { namespace chain {

using vm_type = wasm_interface::vm_type;


class options {
public:
   static options& get() {
      static options* _inst = nullptr;
      if (!_inst) {
         _inst = new options();
      }
      return *_inst;
   }
   int get_wasm_runtime_type();
   ~options();
private:
   options();
   vm_type            wasm_runtime = wasm_interface::vm_type::wabt;

   variables_map & _options;
};


}
}
