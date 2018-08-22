#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <eosiolib_native/vm_api.h>

static struct vm_api s_api;


#include <libethereum/Transaction.h>
#include <libethereum/Executive.h>
#include <libethereum/ChainParams.h>
#include <libethereum/LastBlockHashesFace.h>

#include <libevm/VM.h>
#include <libevm/VMFactory.h>


#include "EosState.h"
#include "EosExecutive.h"
//#include "EosDB.h"

#include <fc/reflect/reflect.hpp>
#include <fc/io/raw.hpp>

#include <eosiolib/types.hpp>

#include <libethcore/SealEngine.h>

using namespace dev;
using namespace dev::eth;

using byte = uint8_t;
using bytes = std::vector<byte>;
std::unique_ptr<dev::eth::SealEngineFace> seal;

int64_t maxBlockGasLimit()
{
//   static int64_t limit = ChainParams(genesisInfo(Network::MainNetworkTest)).maxGasLimit.convert_to<int64_t>();
//   return limit;
   return 0xfffffffff;
}

static std::string const c_genesisInfoMainNetworkNoProofTest = std::string() +
R"E(
{
   "sealEngine": "NoProof",
   "params": {
      "accountStartNonce": "0x00",
      "homesteadForkBlock": "0x118c30",
      "daoHardforkBlock": "0x1d4c00",
      "EIP150ForkBlock": "0x259518",
      "EIP158ForkBlock": "0x28d138",
      "byzantiumForkBlock": "0x2dc6c0",
      "constantinopleForkBlock": "0x2f36ca",
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


class LastBlockHashes: public eth::LastBlockHashesFace
{
public:
   h256s precedingHashes(h256 const& /* _mostRecentHash */) const override { return h256s(256, h256()); }
   void clear() override {}
};


void vm_init(struct vm_api* api) {
   s_api = *api;
   NoProof::init();
   seal = std::unique_ptr<dev::eth::SealEngineFace>(ChainParams(c_genesisInfoMainNetworkNoProofTest).createSealEngine());
}

void vm_deinit() {
   printf("vm_eth finalize\n");
}

struct vm_api* get_vm_api() {
   return &s_api;
}

bool run_code(uint64_t _sender, uint64_t _receiver, bytes& data, bool create);

int vm_setcode(uint64_t account) {
   printf("+++++vm_eth2: setcode\n");
   size_t size = 0;
   const char* code = get_code(account, &size);
   bytes _code(code, code+size);
   run_code(account, account, _code, true);
   return 1;
}

struct EthTransfer {
   uint64_t from;
   uint64_t to;
   uint64_t value;
   std::vector<uint8_t> data;
};

FC_REFLECT( EthTransfer, (from)(to)(value)(data) )


int vm_apply(uint64_t receiver, uint64_t account, uint64_t act) {
   ilog("+++++vm_eth2: apply ${n}", ("n", eosio::name{act}.to_string()));
   if (act != N(transfer)) {
      return 1;
   }

   uint32_t size = action_data_size();
   bytes data(size);
   read_action_data( data.data(), data.size() );

   EthTransfer et;
   fc::raw::unpack<EthTransfer>((char*)data.data(), (uint32_t)data.size(), et);
   require_auth(et.from);
   eosio_assert(et.to == account, "bad receiver");

   run_code(et.from, et.to, et.data, false);

   return 0;
}

int vm_call(uint64_t account, uint64_t func) {
   return 0;
}


static uint64_t g_sender = 0;

void set_sender(uint64_t sender) {
   g_sender = sender;
}

uint64_t get_sender() {
   return g_sender;
}

bool run_code(uint64_t _sender, uint64_t _receiver, bytes& data, bool create)
{
   bytes output;
   Address contractDestination = Address(_receiver);
   Address sender = Address(_sender);
   Address receiver = Address(_receiver);
   Address origin = Address(_receiver);

   set_sender(_sender);

   u256 gas = maxBlockGasLimit();
   u256 gasPrice = 0;
   u256 value = 0;

   StandardTrace st;

   BlockHeader blockHeader; // fake block to be executed in
   blockHeader.setGasLimit(gas);
   blockHeader.setTimestamp(0);

   EosState state;

   Transaction t;
   if (create)
   {
      // Deploy the code on some fake account to be called later.
#if 0
      Account account(0, 0);
      account.setCode(std::vector<uint8_t>{*(reinterpret_cast<dev::bytes*>(&data))});
      std::unordered_map<Address, Account> map;
      map[contractDestination] = account;
      state.populateFrom(map);
#endif
      t = Transaction(value, gasPrice, gas, contractDestination, *(reinterpret_cast<dev::bytes*>(&data)), 0);
      t.forceSender(contractDestination);
   }
   else
   {
      // If not code provided construct "create" transaction out of the input
      // data.
      t = Transaction(value, gasPrice, gas, *(reinterpret_cast<dev::bytes*>(&data)), 0);
      t.forceSender(sender);
   }

//   state.addBalance(sender, value);

   LastBlockHashes lastBlockHashes;
   EnvInfo const envInfo(blockHeader, lastBlockHashes, 0);


   EosExecutive executive(state, envInfo, *seal);

   ExecutionResult res;
   executive.setResultRecipient(res);
   t.forceSender(sender);

   std::unordered_map<byte, std::pair<unsigned, dev::bigint>> counts;
   unsigned total = 0;
   dev::bigint memTotal;

   executive.initialize(t);
   if (create)
       executive.create(sender, value, gasPrice, gas, reinterpret_cast<dev::bytes*>(&data), origin);
   else
       executive.call(contractDestination, sender, value, gasPrice, reinterpret_cast<dev::bytes*>(&data), gas);

   executive.go();

   executive.finalize();

   output.resize( 0 );
   output.resize( res.output.size() );
   memcpy( output.data(), res.output.data(), res.output.size() );

   return true;
}
