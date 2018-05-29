//#include <libethereum/State.h>
#include <libethcore/SealEngine.h>

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

#include "evm/EosState.h"
#include "evm/EosExecutive.h"
#include "evm/EosDB.h"

#include <eosio/chain/evm_interface.hpp>




using namespace dev::eth;
using namespace dev;


namespace eosio {
namespace chain {

evm_interface::evm_interface() {
   init();
}

evm_interface& evm_interface::get() {
   static evm_interface* evm = nullptr;
   if (!evm) {
      evm = new evm_interface();
   }
   return *evm;
}

void evm_interface::init() {
   Ethash::init();
   NoProof::init();
}

void evm_interface::apply(apply_context& c, const shared_string& code) {
   try {
      current_apply_context = &c;
   }FC_CAPTURE_AND_RETHROW()
}

}
}


using byte = uint8_t;
using bytes = std::vector<byte>;


std::unique_ptr<dev::eth::State> globalState;
std::shared_ptr<dev::eth::SealEngineFace> globalSealEngine;

int64_t maxBlockGasLimit()
{
   static int64_t limit = ChainParams(genesisInfo(Network::MainNetworkTest)).maxGasLimit.convert_to<int64_t>();
   return limit;
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

void evm_test_(string _code, string _data);

bool eosio::chain::evm_interface::run_code(apply_context& context, bytes& code, bytes& data, eosio::chain::bytes& output)
{
   Address contractDestination;;
   Address sender;

   if (code.empty()) { //setcode
      auto act = context.act.data_as<eosio::chain::setcode>();
      memcpy(contractDestination.data(), &act.account.value, sizeof(act.account.value));
      ilog( "+++++++++++++act.account:${n}", ("n", act.account.to_string()) );
   } else {
      memcpy(contractDestination.data(), &context.act.account.value, sizeof(context.act.account.value));
      ilog( "+++++++++++++context.act.account:${n}", ("n", context.act.account.to_string()) );
   }
   sender = contractDestination;

   ilog( "+++++++++++++receiver:${n}", ("n", context.receiver.to_string()) );
   ilog( "+++++++++++++account:${n}", ("n", context.act.account.to_string()) );


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

   std::unique_ptr<dev::eth::SealEngineFace> seal = std::unique_ptr<dev::eth::SealEngineFace>(ChainParams(genesisInfo(Network::MainNetworkTest)).createSealEngine());

   EosExecutive executive(state, envInfo, *seal);

   ExecutionResult res;
   executive.setResultRecipient(res);
   t.forceSender(sender);

   std::unordered_map<byte, std::pair<unsigned, dev::bigint>> counts;
   unsigned total = 0;
   dev::bigint memTotal;
   auto onOp = [&](uint64_t step, uint64_t PC, Instruction inst, dev::bigint m, dev::bigint gasCost, dev::bigint gas, dev::eth::VMFace const* evm, ExtVMFace const* extVM) {
//      std::cout << "++++++gasCost: " << gasCost << "\n";
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
   return true;
}

void evm_test_(string _code, string _data)
{
   Network networkName = Network::MainNetworkTest;
   VMKind vmKind = VMKind::Interpreter;
   Mode mode = Mode::Statistics;

   std::vector<uint8_t> code;
   std::vector<uint8_t> data;

   u256 gas = maxBlockGasLimit();
   u256 gasPrice = 0;
   u256 value = 0;

   Ethash::init();
   NoProof::init();

   StandardTrace st;

   BlockHeader blockHeader; // fake block to be executed in
   blockHeader.setGasLimit(gas);
   blockHeader.setTimestamp(0);

   string addr("0xc2ff44dd289190eb47839a3e7bab1ee1abe1ebbe");
   bytesConstRef _input(addr);
//   std::cout<< "dev::ripemd160: " << dev::ripemd160(_input).hex() << std::endl;

   Address sender = Address("0xc2ff44dd289190eb47839a3e7bab1ee1abe1ebbe");
   Address origin = Address(69);

//m_stateDB = State::openDB(_dbPath, bc().genesisHash(), _forceAction);
//m_state(Invalid256, _db, BaseState::PreExisting)
   State state(0);

   code = fromHex(_code);
   data = jsToBytes(_data, OnFailed::Throw);

   Transaction t;
   Address contractDestination("0x7f1d4eef5ce795e6714ea476108aa0d1b519f419");//("0x5fd9151d3eebdfd3d7c12776a8096853804d2b53");
   if (!code.empty())
   {
      // Deploy the code on some fake account to be called later.
      Account account(0, 0);
      account.setCode(std::vector<uint8_t>{code});
      std::unordered_map<Address, Account> map;
      map[contractDestination] = account;
      state.populateFrom(map);
      t = Transaction(value, gasPrice, gas, contractDestination, data, 0);
   }
   else
   {
      // If not code provided construct "create" transaction out of the input
      // data.
      t = Transaction(value, gasPrice, gas, data, 0);
   }

   state.addBalance(sender, value);

   std::unique_ptr<dev::eth::SealEngineFace> se(ChainParams(genesisInfo(networkName)).createSealEngine());
//   std::unique_ptr<SealEngineFace> se(chainParams.createSealEngine());

   LastBlockHashes lastBlockHashes;
   EnvInfo const envInfo(blockHeader, lastBlockHashes, 0);
   Executive executive(state, envInfo, *se);
   ExecutionResult res;
   executive.setResultRecipient(res);
   t.forceSender(sender);

   std::unordered_map<byte, std::pair<unsigned, dev::bigint>> counts;
   unsigned total = 0;
   dev::bigint memTotal;
   auto onOp = [&](uint64_t step, uint64_t PC, Instruction inst, dev::bigint m, dev::bigint gasCost, dev::bigint gas, dev::eth::VMFace const* evm, ExtVMFace const* extVM) {
//      std::cout << "++++++gasCost: " << gasCost << "\n";
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
   };

   executive.initialize(t);
   if (!code.empty())
      executive.call(contractDestination, sender, value, gasPrice, &data, gas);
   else
      executive.create(sender, value, gasPrice, gas, &data, origin);

   Timer timer;
   if ((mode == Mode::Statistics || mode == Mode::Trace) && vmKind == VMKind::Interpreter)
      // If we use onOp, the factory falls back to "interpreter"
      executive.go(onOp);
   else
      executive.go();
   double execTime = timer.elapsed();
   executive.finalize();

   std::vector<uint8_t> output = std::move(res.output);
#if 0
   std::cout << "res.newAddress.hex(): " << res.newAddress.hex() << "\n";
   std::cout << "Gas used: " << res.gasUsed << " (+" << t.baseGasRequired(se->evmSchedule(envInfo.number())) << " for transaction, -" << res.gasRefunded << " refunded)\n";
   std::cout << "Output: " << toHex(output) << "\n";
   std::cout << "toJS(er.output): " << toJS(res.output) << "\n";
   LogEntries logs = executive.logs();
   std::cout << logs.size() << " logs" << (logs.empty() ? "." : ":") << "\n";
   for (LogEntry const& l: logs)
   {
      std::cout << "  " << l.address.hex() << ": " << toHex(t.data()) << "\n";
      for (h256 const& t: l.topics)
         std::cout << "    " << t.hex() << "\n";
   }
#endif
}
