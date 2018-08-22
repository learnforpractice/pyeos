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
#include <libdevcore/Assertions.h>

#include <libethcore/SealEngine.h>

#include <libevm/VMFactory.h>
//#include <libethereum/BlockChain.h>
//#include <libethereum/Block.h>
#include <libethereum/Defaults.h>

#include "EosExtVM.h"

#include <libethereum/TransactionQueue.h>
#include "EosExecutive.h"

#include <fc/log/logger.hpp>
#include <fc/exception/exception.hpp>

#include <eosiolib_native/vm_api.h>

using byte = uint8_t;
using bytes = std::vector<byte>;


namespace eosio {
namespace chain {

#define VM_TYPE 8

bool get_code_size(uint64_t _account, size_t& size) {
   get_vm_api()->get_code(_account, &size);
   return size != 0;
}

}
}

using namespace eosio::chain;

using namespace dev;
using namespace dev::eth;

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
	uint64_t n = ((uint64_t*)_a.data())[0];
	size_t size = 0;
	get_code_size(n, size);
	return size;
}

bool EosState::addressHasCode(Address const& _id) const
{
   size_t size;
   return get_code_size(_id, size);
}

uint64_t get_sender();

void EosState::setStorage(Address const& _contract, u256 const& _key, u256 const& _value)
{
	uint64_t id = 0;
//	m_changeLog.emplace_back(_contract, _key, storage(_contract, _key));
//	m_cache[_contract].setStorage(_key, _value);
    uint64_t n = _contract;

	ilog( "${n1} : ${n2} : ${n3}", ("n1",_key.str())("n2",_value.str())("n3", n) );

	dev::bytes key = dev::toBigEndian(_key);
//FIXME: lost pricision
	memcpy(&id, key.data(), sizeof(id));

	dev::bytes value(32+32);
	value = key;
	value += dev::toBigEndian(_value);
	uint64_t payer = get_sender();
	try {
		for (int i=0;i<key.size()/sizeof(uint64_t);i++) {
			uint64_t id = ((uint64_t*)key.data())[i];
			int itr = db_find_i64( n, n, n, id );
			if (itr < 0) {
				db_store_i64(n, n, payer, id, (const char *)value.data(), value.size() );
				return;
			}
			wlog("update value ${n}", ("n", _value.str()));
			dev::bytes v(32+32);
			int size = db_get_i64( itr, (char*)v.data(), v.size() );
			assert(size == v.size());

			if (memcmp(v.data(), key.data(), key.size()) != 0) {
				//key conflict, find next
				continue;
			}
			db_update_i64( itr, payer, (const char *)value.data(), value.size() );
			return;
		}
	} catch (const fc::exception& e) {
		wlog("exception thrown while call db_store_i64 ${e}", ("e",e.to_detail_string()));
	} catch (std::exception& e) {
      wlog("exception thrown while call db_store_i64 ${e}", ("e",e.what()));
	} catch (...) {
      wlog("unknown exception.");
	}
}

u256 EosState::storage(Address const& _id, u256 const& _key) const
{
	uint64_t id = 0;
	uint64_t n = _id;

	ilog( "${n1} ${n2}", ("n1", _id.hex())("n2", _key.str()) );

	dev::bytes value(32+32);
	memset(value.data(), 0 ,value.size());

	dev::bytes key = dev::toBigEndian(_key);

	try {
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
			assert(size == (32+32));

			if (memcmp(key.data(), value.data(), key.size()) != 0) {
				// key conflict, find next
				continue;
			}
			dev::bytes v(value.begin()+32,value.end());
			u256 ret = dev::fromBigEndian<u256>(v);
			ilog( "got value ${n2}", ("n2", ret.str()) );
			return ret;
		}
	} catch (const fc::exception& e) {
		wlog("exception thrown while call db_get_i64 ${e}", ("e",e.to_detail_string()));
	}

	return 0;
}

h256 EosState::codeHash(Address const& _contract) const {
   elog("fixme:codeHash");
   return h256(0);
}

u256 EosState::balance(Address const& _id) const {
   assert(0);
   return u256(0);
}
