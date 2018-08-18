#pragma once

#include "Precompiled.h"
#include "Address.h"

using namespace dev;
using namespace std;

namespace dev {
namespace eth {
   class PrecompiledContract
   {
   public:
       PrecompiledContract() = default;
       PrecompiledContract(
           PrecompiledPricer const& _cost,
           PrecompiledExecutor const& _exec,
           u256 const& _startingBlock = 0
       ):
           m_cost(_cost),
           m_execute(_exec),
           m_startingBlock(_startingBlock)
       {}
       PrecompiledContract(
           unsigned _base,
           unsigned _word,
           PrecompiledExecutor const& _exec,
           u256 const& _startingBlock = 0
       );

       bigint cost(bytesConstRef _in) const { return m_cost(_in); }
       std::pair<bool, bytes> execute(bytesConstRef _in) const { return m_execute(_in); }

       u256 const& startingBlock() const { return m_startingBlock; }

   private:
       PrecompiledPricer m_cost;
       PrecompiledExecutor m_execute;
       u256 m_startingBlock = 0;
   };

   class ChainParams
   {
   public:
      ChainParams();
      static ChainParams& get() {
         static ChainParams * params = nullptr;
         if (!params) {
            params = new ChainParams();
         }
         return *params;
      }

      std::pair<bool, bytes> executePrecompiled(Address const& _a, bytesConstRef _in, u256 const&) const {
         return precompiled.at(_a).execute(_in);
      }
      /// Precompiled contracts as specified in the chain params.
      std::unordered_map<Address, PrecompiledContract> precompiled;
   };
}
}

