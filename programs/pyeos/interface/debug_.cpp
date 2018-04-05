#include "debug_.hpp"
#include <libdevcore/CommonData.h>
#include <fc/log/logger.hpp>

using namespace dev;

void debug_test() {
   std::string s("abc.py");
   wlog("${n}", ("n", s.find(".py")));
   wlog("${n}", ("n", s.rfind(".py")));

}


void debug_test__() {
   u256 _key = u256(300);
   dev::bytes __key = dev::toBigEndian(_key);
   u256 ___key = dev::fromBigEndian<u256>(__key);
   wlog(___key.str());
}


