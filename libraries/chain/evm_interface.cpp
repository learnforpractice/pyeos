//#include <libethereum/State.h>

#include <libethereum/Transaction.h>
#include <libethereum/Executive.h>
#include <libethereum/ChainParams.h>
#include <libethereum/Block.h>
#include <libethereum/LastBlockHashesFace.h>

#include <libethcore/SealEngine.h>
#include <libdevcore/CommonIO.h>
#include <libdevcore/CommonJS.h>
#include <libdevcore/SHA3.h>

#include <libethashseal/GenesisInfo.h>
#include <libethashseal/Ethash.h>

#include <libevm/VM.h>
#include <libevm/VMFactory.h>

#include <libdevcrypto/Hash.h>

#include <eosio/chain/evm_interface.hpp>


#include "evm/EosState.h"
#include "evm/EosExecutive.h"
#include "evm/EosDB.h"

using namespace dev::eth;
using namespace dev;


namespace eosio {
namespace chain {

evm_interface::evm_interface() {
}

evm_interface& evm_interface::get() {
   static evm_interface* python = nullptr;
   if (!python) {
      wlog("evm_interface::init");
      python = new evm_interface();
   }
   return *python;
}

void evm_interface::apply(apply_context& c, const shared_vector<char>& code) {
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
	static int64_t limit = ChainParams(genesisInfo(Network::MainNetwork)).maxGasLimit.convert_to<int64_t>();
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


void evm_test_(string _code, string _data)
{
	Network networkName = Network::MainNetworkTest;
	VMKind vmKind = VMKind::Interpreter;
	Mode mode = Mode::Statistics;

	bytes code;
	bytes data;

	u256 gas = maxBlockGasLimit();
	u256 gasPrice = 0;
	u256 value = 0;

	Ethash::init();
	NoProof::init();

	StandardTrace st;

	BlockHeader blockHeader; // fake block to be executed in
	blockHeader.setGasLimit(maxBlockGasLimit());
	blockHeader.setTimestamp(0);

	string addr("0xc2ff44dd289190eb47839a3e7bab1ee1abe1ebbe");
	bytesConstRef _input(addr);
	std::cout<< "dev::ripemd160: " << dev::ripemd160(_input).hex() << std::endl;

	Address sender = Address("0xc2ff44dd289190eb47839a3e7bab1ee1abe1ebbe");
	Address origin = Address(69);

//m_stateDB = State::openDB(_dbPath, bc().genesisHash(), _forceAction);
//m_state(Invalid256, _db, BaseState::PreExisting)
	EosDB db;
	EosState state;

//dict: {'to': '0x5fd9151d3eebdfd3d7c12776a8096853804d2b53', 'data':
// '0x6b2fafa9000000000000000000000000000000000000000000000000000000000000000a'}
#if 0
	std::string s("0x0b1bcee7");//("0x6b2fafa9000000000000000000000000000000000000000000000000000000000000000a");
	data = jsToBytes(s, OnFailed::Throw);

	std::string ss("0x6060604052341561000f57600080fd5b600260006040516020015260405180807f6162630000000000000000000000000000000000000000000000000000000000815250600301905060206040518083038160008661646e5a03f1151561006557600080fd5b5050604051805190505060405180807f6162630000000000000000000000000000000000000000000000000000000000815250600301905060405180910390505061017e806100b56000396000f30060606040526004361061004c576000357c0100000000000000000000000000000000000000000000000000000000900463ffffffff1680630b1bcee7146100515780636b2fafa91461007a575b600080fd5b341561005c57600080fd5b6100646100b9565b6040518082815260200191505060405180910390f35b341561008557600080fd5b61009b60048080359060200190919050506100c1565b60405180826000191660001916815260200191505060405180910390f35b600044905090565b6000600260006040516020015260405180807f2b2b2b2b2b2b2b2b2b2b2b2b2b2b2b2b2b2b2b68656c6c6f2c20736d6172742081526020017f636f6e7472616374206162636465660000000000000000000000000000000000815250602f01905060206040518083038160008661646e5a03f1151561013f57600080fd5b50506040518051905050814090509190505600a165627a7a72305820c822a73ad2c7085b271c5b119aad8b267b2f31effc8cb6252369049db11376d40029");
	code = fromHex(ss);
#else
	std::string s("0x6060604052341561000f57600080fd5b600260006040516020015260405180807f6162630000000000000000000000000000000000000000000000000000000000815250600301905060206040518083038160008661646e5a03f1151561006557600080fd5b5050604051805190505060405180807f6162630000000000000000000000000000000000000000000000000000000000815250600301905060405180910390505061017e806100b56000396000f30060606040526004361061004c576000357c0100000000000000000000000000000000000000000000000000000000900463ffffffff1680630b1bcee7146100515780636b2fafa91461007a575b600080fd5b341561005c57600080fd5b6100646100b9565b6040518082815260200191505060405180910390f35b341561008557600080fd5b61009b60048080359060200190919050506100c1565b60405180826000191660001916815260200191505060405180910390f35b600044905090565b6000600260006040516020015260405180807f2b2b2b2b2b2b2b2b2b2b2b2b2b2b2b2b2b2b2b68656c6c6f2c20736d6172742081526020017f636f6e7472616374206162636465660000000000000000000000000000000000815250602f01905060206040518083038160008661646e5a03f1151561013f57600080fd5b50506040518051905050814090509190505600a165627a7a72305820c822a73ad2c7085b271c5b119aad8b267b2f31effc8cb6252369049db11376d40029");
	data = jsToBytes(s, OnFailed::Throw);
//'0x6b2fafa9000000000000000000000000000000000000000000000000000000000000000a'
//'0x6060604052341561000f57600080fd5b600260006040516020015260405180807f6162630000000000000000000000000000000000000000000000000000000000815250600301905060206040518083038160008661646e5a03f1151561006557600080fd5b5050604051805190505060405180807f6162630000000000000000000000000000000000000000000000000000000000815250600301905060405180910390505061017e806100b56000396000f30060606040526004361061004c576000357c0100000000000000000000000000000000000000000000000000000000900463ffffffff1680630b1bcee7146100515780636b2fafa91461007a575b600080fd5b341561005c57600080fd5b6100646100b9565b6040518082815260200191505060405180910390f35b341561008557600080fd5b61009b60048080359060200190919050506100c1565b60405180826000191660001916815260200191505060405180910390f35b600044905090565b6000600260006040516020015260405180807f2b2b2b2b2b2b2b2b2b2b2b2b2b2b2b2b2b2b2b68656c6c6f2c20736d6172742081526020017f636f6e7472616374206162636465660000000000000000000000000000000000815250602f01905060206040518083038160008661646e5a03f1151561013f57600080fd5b50506040518051905050814090509190505600a165627a7a72305820c822a73ad2c7085b271c5b119aad8b267b2f31effc8cb6252369049db11376d40029'
//0x0b1bcee7
#endif

	code = fromHex(_code);
	data = jsToBytes(_data, OnFailed::Throw);

	Transaction t;
	Address contractDestination("0x7f1d4eef5ce795e6714ea476108aa0d1b519f419");//("0x5fd9151d3eebdfd3d7c12776a8096853804d2b53");
	if (!code.empty())
	{
		// Deploy the code on some fake account to be called later.
		Account account(0, 0);
		account.setCode(bytes{code});
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

	auto configJSON = contentsString("/Users/newworld/dev/raiblocks/build/genesis.json");

	ChainParams chainParams;
	chainParams = chainParams.loadConfig(configJSON);

	std::unique_ptr<SealEngineFace> se(ChainParams(genesisInfo(networkName)).createSealEngine());
//	std::unique_ptr<SealEngineFace> se(chainParams.createSealEngine());

	LastBlockHashes lastBlockHashes;
	EnvInfo const envInfo(blockHeader, lastBlockHashes, 0);
	EosExecutive executive(state, envInfo, *se);
	ExecutionResult res;
	executive.setResultRecipient(res);
	t.forceSender(sender);

	std::unordered_map<byte, std::pair<unsigned, bigint>> counts;
	unsigned total = 0;
	bigint memTotal;
	auto onOp = [&](uint64_t step, uint64_t PC, Instruction inst, bigint m, bigint gasCost, bigint gas, dev::eth::VM* evm, ExtVMFace const* extVM) {
//		std::cout << "++++++gasCost: " << gasCost << "\n";
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
	printf("++++++++executive.finalize()\n");

	bytes output = std::move(res.output);

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

}
