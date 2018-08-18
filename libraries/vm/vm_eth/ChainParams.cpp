#include "ChainParams.h"

namespace dev {
namespace eth {

PrecompiledContract::PrecompiledContract(
    unsigned _base,
    unsigned _word,
    PrecompiledExecutor const& _exec,
    u256 const& _startingBlock
):
    PrecompiledContract([=](bytesConstRef _in) -> bigint
    {
        bigint s = _in.size();
        bigint b = _base;
        bigint w = _word;
        return b + (s + 31) / 32 * w;
    }, _exec, _startingBlock)
{}


ChainParams::ChainParams() {
   precompiled.insert(make_pair(Address(1), PrecompiledContract(3000, 0, PrecompiledRegistrar::executor("ecrecover"))));
   precompiled.insert(make_pair(Address(2), PrecompiledContract(60, 12, PrecompiledRegistrar::executor("sha256"))));
   precompiled.insert(make_pair(Address(3), PrecompiledContract(600, 120, PrecompiledRegistrar::executor("ripemd160"))));
   precompiled.insert(make_pair(Address(4), PrecompiledContract(15, 3, PrecompiledRegistrar::executor("identity"))));
}

}
}


