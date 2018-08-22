#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <eosiolib_native/vm_api.h>

static struct vm_api s_api;


#include <libethereum/Transaction.h>
#include <libethereum/Executive.h>
#include <libethereum/ChainParams.h>
#include <libethereum/Block.h>
#include <libethereum/LastBlockHashesFace.h>

#include <libdevcore/CommonIO.h>
#include <libdevcore/CommonJS.h>
#include <libdevcore/SHA3.h>

#include <libethashseal/GenesisInfo.h>
#include <libethashseal/Ethash.h>

#include <libevm/VM.h>
#include <libevm/VMFactory.h>

#include <libdevcrypto/Hash.h>

#include "EosState.h"
#include "EosExecutive.h"
//#include "EosDB.h"

using namespace dev::eth;
using namespace dev;

using byte = uint8_t;
using bytes = std::vector<byte>;


std::unique_ptr<dev::eth::State> globalState;
std::shared_ptr<dev::eth::SealEngineFace> globalSealEngine;

int64_t maxBlockGasLimit()
{
//   static int64_t limit = ChainParams(genesisInfo(Network::MainNetworkTest)).maxGasLimit.convert_to<int64_t>();
//   return limit;
   return 0xfffffffff;
}


enum class Mode
{
   Trace,
   Statistics,
   OutputOnly,

   /// Test mode -- output information needed for test verification and
   /// benchmarking. The execution is not introspected not to degrade
   /// performance.
   Test
};

class LastBlockHashes: public eth::LastBlockHashesFace
{
public:
   h256s precedingHashes(h256 const& /* _mostRecentHash */) const override { return h256s(256, h256()); }
   void clear() override {}
};


void vm_init(struct vm_api* api) {
   s_api = *api;
}

void vm_deinit() {
   printf("vm_eth finalize\n");
}

struct vm_api* get_vm_api() {
   return &s_api;
}

int vm_setcode(uint64_t account) {
   printf("+++++vm_eth: setcode\n");
   return 0;
}

int vm_apply(uint64_t receiver, uint64_t account, uint64_t act) {
   printf("+++++vm_eth: apply\n");
   return 0;
}

int vm_call(uint64_t account, uint64_t func) {
   return 0;
}

static std::string const c_genesisInfoMainNetworkTest = std::string() +
R"E(
{
   "sealEngine": "Ethash",
   "params": {
      "accountStartNonce": "0x00",
      "homesteadForkBlock": "0x118c30",
      "daoHardforkBlock": "0x1d4c00",
      "EIP150ForkBlock": "0x259518",
      "EIP158ForkBlock": "0x28d138",
      "byzantiumForkBlock": "0x42ae50",
      "constantinopleForkBlock": "0x500000",
      "networkID" : "0x01",
      "chainID": "0x01",
      "maximumExtraDataSize": "0x20",
      "tieBreakingGas": false,
      "minGasLimit": "0x1388",
      "maxGasLimit": "7fffffffffffffff",
      "gasLimitBoundDivisor": "0x0400",
      "minimumDifficulty": "0x020000",
      "difficultyBoundDivisor": "0x0800",
      "durationLimit": "0x0d",
      "blockReward": "0x4563918244F40000"
   },
   "genesis": {
      "nonce": "0x0000000000000042",
      "difficulty": "0x020000",
      "mixHash": "0x0000000000000000000000000000000000000000000000000000000000000000",
      "author": "0x0000000000000000000000000000000000000000",
      "timestamp": "0x00",
      "parentHash": "0x0000000000000000000000000000000000000000000000000000000000000000",
      "extraData": "0x11bbe8db4e347b4e8c937c1c8370e4b5ed33adb3db69cbdb7a38e1e50b1b82fa",
      "gasLimit": "0x1388"
   },
   "accounts": {
      "0000000000000000000000000000000000000001": { "precompiled": { "name": "ecrecover", "linear": { "base": 3000, "word": 0 } } },
      "0000000000000000000000000000000000000002": { "precompiled": { "name": "sha256", "linear": { "base": 60, "word": 12 } } },
      "0000000000000000000000000000000000000003": { "precompiled": { "name": "ripemd160", "linear": { "base": 600, "word": 120 } } },
      "0000000000000000000000000000000000000004": { "precompiled": { "name": "identity", "linear": { "base": 15, "word": 3 } } },
      "0000000000000000000000000000000000000005": { "precompiled": { "name": "modexp", "startingBlock" : "0x2dc6c0" } },
      "0000000000000000000000000000000000000006": { "precompiled": { "name": "alt_bn128_G1_add", "startingBlock" : "0x2dc6c0", "linear": { "base": 500, "word": 0 } } },
      "0000000000000000000000000000000000000007": { "precompiled": { "name": "alt_bn128_G1_mul", "startingBlock" : "0x2dc6c0", "linear": { "base": 40000, "word": 0 } } },
      "0000000000000000000000000000000000000008": { "precompiled": { "name": "alt_bn128_pairing_product", "startingBlock" : "0x2dc6c0" } }
   }
}
)E";

bool run_code(uint64_t account, bytes& code, bytes& data, bytes& output)
{
   Address contractDestination;;
   Address sender;
   contractDestination = Address(account);

   sender = contractDestination;


   VMKind vmKind = VMKind::Interpreter;
   Mode mode = Mode::Statistics;

   u256 gas = maxBlockGasLimit();
   u256 gasPrice = 0;
   u256 value = 0;



   StandardTrace st;

   BlockHeader blockHeader; // fake block to be executed in
   blockHeader.setGasLimit(gas);
   blockHeader.setTimestamp(0);


   Address origin = contractDestination;//Address(69);


   EosState state;
//   State state(0);

//   code = dev::fromHex(_code);
//   code = std::vector<uint8_t>( _code.begin(), _code.end() );
//   data = dev::jsToBytes(_data, OnFailed::Throw);

   Transaction t;
   if (!code.empty())
   {
      // Deploy the code on some fake account to be called later.
      Account account(0, 0);
      account.setCode(std::vector<uint8_t>{*(reinterpret_cast<dev::bytes*>(&code))});
      std::unordered_map<Address, Account> map;
      map[contractDestination] = account;
      state.populateFrom(map);
      t = Transaction(value, gasPrice, gas, contractDestination, *(reinterpret_cast<dev::bytes*>(&data)), 0);
   }
   else
   {
      // If not code provided construct "create" transaction out of the input
      // data.
      t = Transaction(value, gasPrice, gas, *(reinterpret_cast<dev::bytes*>(&data)), 0);
   }

   state.addBalance(sender, value);

   LastBlockHashes lastBlockHashes;
   EnvInfo const envInfo(blockHeader, lastBlockHashes, 0);

   std::unique_ptr<dev::eth::SealEngineFace> seal = std::unique_ptr<dev::eth::SealEngineFace>(ChainParams(c_genesisInfoMainNetworkTest).createSealEngine());

   EosExecutive executive(state, envInfo, *seal);

   ExecutionResult res;
   executive.setResultRecipient(res);
   t.forceSender(sender);

   std::unordered_map<byte, std::pair<unsigned, dev::bigint>> counts;
   unsigned total = 0;
   dev::bigint memTotal;
   auto onOp = [&](uint64_t step, uint64_t PC, Instruction inst, dev::bigint m, dev::bigint gasCost, dev::bigint gas, dev::eth::VMFace const* evm, ExtVMFace const* extVM) {
//      std::cout << "++++++gasCost: " << gasCost << "\n";
#if 0
      if (mode == Mode::Statistics)
      {
         counts[(byte)inst].first++;
         counts[(byte)inst].second += gasCost;
         total++;
         if (m > 0)
            memTotal = m;
      }
      else if (mode == Mode::Trace)
         st(step, PC, inst, m, gasCost, gas, evm, extVM);
#endif
   };

   executive.initialize(t);
   if (!code.empty())
      executive.call(contractDestination, sender, value, gasPrice, reinterpret_cast<dev::bytes*>(&data), gas);
   else
      executive.create(sender, value, gasPrice, gas, reinterpret_cast<dev::bytes*>(&data), origin);

   Timer timer;
   if ((mode == Mode::Statistics || mode == Mode::Trace) && vmKind == VMKind::Interpreter)
      // If we use onOp, the factory falls back to "interpreter"
      executive.go(onOp);
   else
      executive.go();
   double execTime = timer.elapsed();
   executive.finalize();

   output.resize( 0 );
   output.resize( res.output.size() );
   memcpy( output.data(), res.output.data(), res.output.size() );

#if 0
   std::cout << "res.newAddress.hex(): " << res.newAddress.hex() << "\n";
   std::cout << "Gas used: " << res.gasUsed << " (+" << t.baseGasRequired(seal->evmSchedule(envInfo.number())) << " for transaction, -" << res.gasRefunded << " refunded)\n";
   std::cout << "res.output.size():" << res.output.size() << "\n";
   std::cout << "Output: " << toHex(res.output) << "\n";
//   std::cout << "toJS(res.output): " << toJS(res.output) << "\n";

   LogEntries logs = executive.logs();
   std::cout << logs.size() << " logs" << (logs.empty() ? "." : ":") << "\n";
   for (LogEntry const& l: logs)
   {
      std::cout << "  " << l.address.hex() << ": " << toHex(t.data()) << "\n";
      for (h256 const& t: l.topics)
         std::cout << "    " << t.hex() << "\n";
   }
#endif
   return true;
}
