#include "debug_.hpp"
#include <libdevcore/CommonData.h>
#include <fc/log/logger.hpp>

using namespace dev;

void debug_test() {
	u256 _key = u256(300);
	dev::bytes __key = dev::toBigEndian(_key);
	u256 ___key = dev::fromBigEndian<u256>(__key);
	wlog(___key.str());
}


