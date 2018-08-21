/*
    This file is part of cpp-ethereum.

    cpp-ethereum is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    cpp-ethereum is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file State.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#include "State.h"


#include "ExtVM.h"
#include <VMFactory.h>
#include <boost/filesystem.hpp>
#include <boost/timer.hpp>

#include <eosiolib/db.h>
#include <eosiolib/system.h>

using namespace std;
using namespace dev;
using namespace dev::eth;
namespace fs = boost::filesystem;

bool State::addressInUse(Address const& _id) const
{
    assert(0);
    return false;
}

bool State::addressHasCode(Address const& _id) const
{
   size_t size = 0;
   get_code(_id, &size);
   return size != 0;
}

u256 State::balance(Address const& _id) const
{
   assert(false);
   return 0;
}

void State::addBalance(Address const& _id, u256 const& _amount)
{

}

void State::subBalance(Address const& _addr, u256 const& _value)
{

}

void State::setBalance(Address const& _addr, u256 const& _value)
{

}

void State::createContract(Address const& _address)
{
//    createAccount(_address, {requireAccountStartNonce(), 0});
}

void State::kill(Address _addr)
{

}

u256 State::storage(Address const& _id, u256 const& _key) const
{
   uint64_t id = 0;
   uint64_t n = _id;

// ilog( "${n1} ${n2}", ("n1", _id.hex())("n2", _key.str()) );

   dev::bytes value(32+32);
   memset(value.data(), 0 ,value.size());

   dev::bytes key = dev::toBigEndian(_key);

   printf("storage: %s\n", _key.str().c_str());

   for (int i=0;i<key.size()/sizeof(uint64_t);i++) {
      uint64_t id = ((uint64_t*)key.data())[i];
      int itr = db_find_i64( n, n, n, id );
      if (itr < 0) {
         return 0;
      }

      int size = db_get_i64( itr, (char*)value.data(), value.size() );
      if (size <= 0) {
         return 0;
      }

      eosio_assert(size == (32+32), "bad storage");

      if (memcmp(key.data(), value.data(), key.size()) != 0) {
         // key conflict, find next
         continue;
      }
      dev::bytes v(value.begin()+32,value.end());
      u256 ret = dev::fromBigEndian<u256>(v);
//      ilog( "got value ${n2}", ("n2", ret.str()) );
      return ret;
   }

   return 0;

}

void State::setStorage(Address const& _contract, u256 const& _key, u256 const& _value)
{
   uint64_t id = 0;
   uint64_t n = _contract;

// ilog( "${n1} : ${n2} : ${n3}", ("n1",_key.str())("n2",_value.str())("n3", n) );
   printf("setStorage: %s %s\n", _key.str().c_str(), _value.str().c_str());
   dev::bytes key = dev::toBigEndian(_key);

   dev::bytes value(32+32);
   value = key;
   value += dev::toBigEndian(_value);

   for (int i=0;i<key.size()/sizeof(uint64_t);i++) {
      uint64_t id = ((uint64_t*)key.data())[i];
      int itr = db_find_i64( n, n, n, id );
      if (itr < 0) {
         db_store_i64(n, n, n, id, (const char *)value.data(), value.size() );
         return;
      }
//         wlog("update value ${n}", ("n", _value.str()));
      dev::bytes v(32+32);
      int size = db_get_i64( itr, (char*)v.data(), v.size() );

      eosio_assert(size == v.size(), "bad storage");

      if (memcmp(v.data(), key.data(), key.size()) != 0) {
         //key conflict, find next
         continue;
      }
      db_update_i64( itr, n, (const char *)value.data(), value.size() );
      return;
   }

}

void State::clearStorage(Address const& _contract)
{
//   assert(0);
}

h256 State::storageRoot(Address const& _id) const
{
   return h256(0);
}

bytes const& State::code(Address const& _addr) const
{
   uint64_t receiver = _addr;
   size_t size;

   eosio_assert (get_code_type(_addr) == 2, "bad vm type");

   const char *code = get_code( receiver, &size );
   return bytes(code, code+size);
}

void State::setCode(Address const& _address, bytes&& _code)
{
   set_code(_address, 2, (char*)_code.data(), _code.size());
}

h256 State::codeHash(Address const& _a) const
{
   h256 code_id;
   get_code_id( _a, (char*)code_id.data(), code_id.size );
   return code_id;//EmptySHA3;
}

size_t State::codeSize(Address const& _a) const
{
   size_t size;
   get_code( _a, &size );
   return size;
}

#if 0
bool State::execute(EnvInfo const& _envInfo, Transaction const& _t, Permanence _p, OnOpFunc const& _onOp, bytes& output)
{
    // Create and initialize the executive. This will throw fairly cheaply and quickly if the
    // transaction is bad in any way.
    Executive e(*this, _envInfo);
    ExecutionResult res;
    e.setResultRecipient(res);

    auto onOp = _onOp;
    u256 const startGasUsed = _envInfo.gasUsed();
    bool const statusCode = executeTransaction(e, _t, onOp);

    bool removeEmptyAccounts = false;
    switch (_p)
    {
        case Permanence::Reverted:
            m_cache.clear();
            break;
        case Permanence::Committed:
            removeEmptyAccounts = _envInfo.number() >= _sealEngine.chainParams().EIP158ForkBlock;
            commit(removeEmptyAccounts ? State::CommitBehaviour::RemoveEmptyAccounts : State::CommitBehaviour::KeepEmptyAccounts);
            break;
        case Permanence::Uncommitted:
            break;
    }

    return make_pair(res, receipt);
}
#endif

State& State::operator=(State const& _s)
{
    if (&_s == this)
        return *this;
    return *this;
}
