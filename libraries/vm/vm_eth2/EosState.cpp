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
/** @file EosState.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#include "EosState.h"

#include <boost/filesystem.hpp>
#include <boost/timer.hpp>

#include <libevm/VMFactory.h>
//#include <libethereum/BlockChain.h>
//#include <libethereum/Block.h>

#include "EosExtVM.h"

#include "EosExecutive.h"

#include <fc/log/logger.hpp>
#include <fc/exception/exception.hpp>

#include <eosiolib_native/vm_api.h>
#include <eosiolib/db.h>
#include <eosiolib/symbol.hpp>

using byte = uint8_t;
using bytes = std::vector<byte>;


namespace eosio {
namespace chain {

#define VM_TYPE 2

bool get_code_size(uint64_t _account, size_t& size) {
   get_vm_api()->get_code(_account, &size);
   return size != 0;
}

}
}

using namespace eosio::chain;

using namespace dev;
using namespace dev::eth;

static uint64_t EOS = S(4, EOS);

void EosState::transferBalance(Address const& _from, Address const& _to, u256 const& _value) {
   int64_t value = _value.convert_to<int64_t>();
   if (value == 0) {
      return;
   }
   uint64_t from = _from;
   uint64_t to = _to;
   uint64_t receiver = get_vm_api()->current_receiver();
   if (from == receiver) {
      get_vm_api()->transfer_inline(_to, value, EOS);
   } else {
      get_vm_api()->transfer(from, to, value, EOS);
   }
}

bool EosState::addressInUse(Address const& _id) const
{
   return false;
   return is_account(_id);
}


bytes const& EosState::code(Address const& _addr) const {
   static bytes _code;
   uint64_t receiver = _addr;
   size_t size=0;

   eosio_assert (get_code_type(_addr) == VM_TYPE, "bad vm type");
   const char *code = get_code( receiver, &size );

   _code.resize(size);
   memcpy(_code.data(), code, size);
   return _code;
}

/// Sets the code of the account. Must only be called during / after contract creation.
void EosState::setCode(Address const& _address, bytes&& _code) {
   set_code(_address, VM_TYPE, (char*)_code.data(), _code.size());
}


size_t EosState::codeSize(Address const& _a) const
{
    uint64_t n = _a;
	size_t size = 0;
	get_code_size(n, size);
	return size;
}

bool EosState::addressHasCode(Address const& _id) const
{
   if (VM_TYPE_ETH != get_code_type(_id)) {
      return false;
   }
   size_t size = 0;
   return get_code_size(_id, size);
}

uint64_t get_sender();

void EosState::setStorage(Address const& _contract, u256 const& _key, u256 const& _value)
{
//	m_changeLog.emplace_back(_contract, _key, storage(_contract, _key));
//	m_cache[_contract].setStorage(_key, _value);
    uint64_t n = _contract;
	ilog( "${n1} : ${n2} : ${n3}", ("n1",_key.str())("n2",_value.str())("n3", n) );

	dev::bytes key = dev::toBigEndian(_key);

	dev::bytes value = dev::toBigEndian(_value);
	uint64_t payer = get_sender();

   int itr = db_find_i256( n, n, n, key.data(), key.size() );
   if (itr < 0) {
      if (_value == 0) {

      } else {
         db_store_i256(n, n, n, payer, key.data(), key.size(), (const char *)value.data(), value.size() );
      }
      return;
   }

   if (_value == 0) {
      db_remove_i256(itr);
   } else {
      db_update_i256( itr, payer, (const char *)value.data(), value.size() );
   }
   return;
}

u256 EosState::storage(Address const& _id, u256 const& _key) const
{
	uint64_t n = _id;
	auto temp = dev::toBigEndian(_key);
//	ilog( "${n1} ${n2}", ("n1", _id.hex())("n2", *(reinterpret_cast<bytes*>(&temp))) );

	dev::bytes value(32);
	memset(value.data(), 0 ,value.size());

	dev::bytes key = dev::toBigEndian(_key);
	int itr = db_find_i256(n, n, n, key.data(), key.size());
	if (itr < 0) {
	   return 0;
	}

	int size = db_get_i256( itr, (char *)value.data(), value.size() );
	eosio_assert(size == 32, "bad storage");

   u256 ret = dev::fromBigEndian<u256>(value);
//   wlog( "got value ${n2}", ("n2", ret.str()) );
   return ret;
}

h256 EosState::codeHash(Address const& _contract) const {
   h256 code_id(0);
   get_code_id( _contract, (char*)code_id.data(), code_id.size );
   return code_id;//EmptySHA3;
}

u256 EosState::balance(Address const& _id) const {
   int64_t amount = 0;
   //534f4504: 4,EOS
   get_vm_api()->get_balance(_id, 0x534f4504, &amount);
   return u256(amount);
}
