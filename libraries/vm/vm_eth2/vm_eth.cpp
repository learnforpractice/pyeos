#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <eosiolib_native/vm_api.h>

static struct vm_api s_api;


#include <libethereum/Transaction.h>
#include <libethereum/Executive.h>
#include <libethereum/ChainParams.h>
#include <libethereum/LastBlockHashesFace.h>

#include <libevm/VMFactory.h>


#include "EosState.h"
#include "EosExecutive.h"
//#include "EosDB.h"

#include <fc/reflect/reflect.hpp>
#include <fc/io/raw.hpp>
#include <fc/crypto/hex.hpp>

#include <eosiolib/types.hpp>

#include <libethcore/SealEngine.h>

#include <eosiolib/action.hpp>
#include <eosiolib/currency.hpp>

using namespace eosio;
using namespace dev;
using namespace dev::eth;

class LastBlockHashes: public eth::LastBlockHashesFace
{
public:
   h256s precedingHashes(h256 const& /* _mostRecentHash */) const override { return h256s(256, h256()); }
   void clear() override {}
};

static std::unique_ptr<dev::eth::SealEngineFace> seal;

static BlockHeader blockHeader;
static LastBlockHashes lastBlockHashes;

static std::unique_ptr<dev::eth::EnvInfo> envInfo;
static EosState state;

int64_t maxBlockGasLimit()
{
//   static int64_t limit = ChainParams(genesisInfo(Network::MainNetworkTest)).maxGasLimit.convert_to<int64_t>();
//   return limit;
   return 0xfffffffffffffff;
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


uint64_t ethaddr2n(const char* address, int size) {
   eosio_assert(size == Address::size, "wrong address size");
   dev::bytes b(address, address+size);
   return Address(b);
}

void n2ethaddr(uint64_t n, char* address, int size) {
   eosio_assert(size == Address::size, "wrong address size");
   Address a(n);
   memcpy(address, a.data(), Address::size);
}

int sha3_(const char* data, int size, char* result, int size2) {
   if (size2 != h256::size) {
      return 0;
   }
   dev::bytes bs(data, data+size);
   h256 ret = dev::sha3(bs);
   memcpy(result, ret.data(), h256::size);
   return h256::size;
}

void vm_init(struct vm_api* api) {
   api->ethaddr2n = ethaddr2n;
   api->n2ethaddr = n2ethaddr;
   api->sha3 = sha3_;

   s_api = *api;
   NoProof::init();
   seal = std::unique_ptr<dev::eth::SealEngineFace>(ChainParams(c_genesisInfoMainNetworkNoProofTest).createSealEngine());
   blockHeader.setNumber(0x42ae50);//byzantiumForkBlock
   envInfo = std::make_unique<dev::eth::EnvInfo>(blockHeader, lastBlockHashes, 0);
}

void vm_deinit() {
   printf("vm_eth finalize\n");
}

bool run_code(uint64_t _sender, uint64_t _receiver, int64_t _value, dev::bytes& data, bool create, bool transfer);

int vm_setcode(uint64_t account) {
   printf("+++++vm_eth2: setcode\n");
   size_t size = 0;
   const char* code = get_code(account, &size);
   if (size <= 0) {
      return 1;
   }
   dev::bytes _code(code, code+size);
   run_code(account, account, 0,  _code, true, false);
   return 1;
}

struct EthTransfer {
   uint64_t from;
   uint64_t to;
   int64_t value;
   std::vector<uint8_t> data;
};

FC_REFLECT( EthTransfer, (from)(to)(value)(data) )


int vm_apply(uint64_t receiver, uint64_t account, uint64_t act) {
//   ilog("+++++vm_eth2: apply ${n}", ("n", eosio::name{act}.to_string()));
   if (act == N(transfer)) {
      if (account == N(eosio.token)) {
         auto transfer = unpack_action_data<currency::transfer>();
         eosio_assert(transfer.quantity.symbol == S(4, EOS), "not EOS token");
         eosio_assert(transfer.quantity.is_valid(), "invalid transfer");
         eosio_assert(transfer.quantity.amount > 0, "invalid amount");
   //      dev::bytes data(transfer.memo.begin(), transfer.memo.end());
         eosio_assert(transfer.memo.size() % 2 == 0, "invalid hex string");
         eosio::bytes data(transfer.memo.size()/2);
         fc::from_hex(transfer.memo, data.data(), data.size());
         run_code(transfer.from, transfer.to, transfer.quantity.amount, *reinterpret_cast<dev::bytes*>(&data), false, false);
      } else {
         uint32_t size = action_data_size();
         eosio::bytes data(size);
         read_action_data( data.data(), data.size() );

         EthTransfer et;
         fc::raw::unpack<EthTransfer>((char*)data.data(), (uint32_t)data.size(), et);
         require_auth(et.from);
         eosio_assert(et.to == account, "bad receiver");

         run_code(et.from, et.to, et.value, et.data, false, true);
         return 1;
      }
   }
   return 0;
}

int vm_call(uint64_t account, uint64_t func) {
   return 0;
}


int vm_preload(uint64_t account) {
   return 0;
}

int vm_unload(uint64_t account) {
   return 0;
}

static uint64_t g_sender = 0;

void set_sender(uint64_t sender) {
   g_sender = sender;
}

uint64_t get_sender() {
   return g_sender;
}

bool run_code(uint64_t _sender, uint64_t _receiver, int64_t _value, dev::bytes& data, bool create, bool transfer)
{
   dev::bytes output;
   Address contractDestination = Address(_receiver);
   Address sender = Address(_sender);
//   Address receiver = Address(_receiver);
   Address origin = Address(_receiver);

   set_sender(_sender);

   u256 gas = maxBlockGasLimit();
   u256 gasPrice = 0;
   u256 value(_value);

   Transaction t;
   if (create)
   {
      t = Transaction(value, gasPrice, gas, data, 0);
      t.forceSender(sender);
   }
   else
   {
       t = Transaction(value, gasPrice, gas, contractDestination, data, 0);
       t.forceSender(contractDestination);
   }

   EosExecutive executive(state, *envInfo, *seal);

//   ExecutionResult res;
//   executive.setResultRecipient(res);
   t.forceSender(sender);

   executive.initialize(t);
#if 0
   executive.execute();
#else
   if (create)
       executive.create(sender, value, gasPrice, gas, &data, origin);
   else
       executive.call(contractDestination, sender, value, gasPrice, &data, gas, transfer);
#endif

   executive.go();

   executive.finalize();

//   output.resize( 0 );
//   output.resize( res.output.size() );
//   memcpy( output.data(), res.output.data(), res.output.size() );

   return true;
}
