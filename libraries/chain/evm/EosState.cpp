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
#include <libdevcore/TrieHash.h>
#include <libdevcore/TrieDB.h>

#include <libethcore/SealEngine.h>

#include <libevm/VMFactory.h>
#include <libethereum/BlockChain.h>
#include <libethereum/Block.h>
#include <libethereum/Defaults.h>
#include <libethereum/ExtVM.h>
#include <libethereum/TransactionQueue.h>
#include <libethereum/Executive.h>

#include <fc/log/logger.hpp>

namespace eosio {
namespace chain {

bool get_code(uint64_t _account, std::vector<uint8_t>& v);
bool get_code_size(uint64_t _account, int& size);

}
}

using namespace eosio::chain;

extern "C" {
int store_str(uint64_t scope, uint64_t table, const char* key, uint32_t key_len, const char* data, size_t data_len);
int update_str(uint64_t scope, uint64_t table, const char* key, uint32_t key_len, const char* data, size_t data_len);
int remove_str(uint64_t scope, uint64_t table, const char* key, uint32_t key_len);

int load_str(uint64_t scope, uint64_t code, uint64_t table, const char* key, size_t key_len, char* data, size_t data_len);
int front_str(uint64_t scope, uint64_t code, uint64_t table, const char* key, size_t key_len, char* data, size_t data_len);
int back_str(uint64_t scope, uint64_t code, uint64_t table, const char* key, size_t key_len, char* data, size_t data_len);
int next_str(uint64_t scope, uint64_t code, uint64_t table, const char* key, size_t key_len, char* data, size_t data_len);
int previous_str(uint64_t scope, uint64_t code, uint64_t table, const char* key, size_t key_len, char* data, size_t data_len);
int lower_bound_str(uint64_t scope, uint64_t code, uint64_t table, const char* key, size_t key_len, char* data, size_t data_len);
int upper_bound_str(uint64_t scope, uint64_t code, uint64_t table, const char* key, size_t key_len, char* data, size_t data_len);

}


using namespace dev::eth;

#if 0
void EosState::setCode(Address const& _address, bytes&& _code)
{
	m_changeLog.emplace_back(_address, code(_address));
	m_cache[_address].setCode(std::move(_code));
}

void EosState::clearStorage(Address const& _contract)
{
	h256 const& oldHash{m_cache[_contract].baseRoot()};
	if (oldHash == EmptyTrie)
		return;
	m_changeLog.emplace_back(Change::StorageRoot, _contract, oldHash);
	m_cache[_contract].clearStorage();
}

void EosState::addBalance(Address const& _id, u256 const& _amount)
{
	if (Account* a = account(_id))
	{
		// Log empty account being touched. Empty touched accounts are cleared
		// after the transaction, so this event must be also reverted.
		// We only log the first touch (not dirty yet), and only for empty
		// accounts, as other accounts does not matter.
		// TODO: to save space we can combine this event with Balance by having
		//       Balance and Balance+Touch events.
		if (!a->isDirty() && a->isEmpty())
			m_changeLog.emplace_back(Change::Touch, _id);

		// Increase the account balance. This also is done for value 0 to mark
		// the account as dirty. Dirty account are not removed from the cache
		// and are cleared if empty at the end of the transaction.
		a->addBalance(_amount);
	}
	else
		createAccount(_id, {requireAccountStartNonce(), _amount});

	if (_amount)
		m_changeLog.emplace_back(Change::Balance, _id, _amount);
}

void EosState::rollback(size_t _savepoint)
{
	while (_savepoint != m_changeLog.size())
	{
		auto& change = m_changeLog.back();
		auto& account = m_cache[change.address];

		// Public State API cannot be used here because it will add another
		// change log entry.
		switch (change.kind)
		{
		case Change::Storage:
			account.setStorage(change.key, change.value);
			break;
		case Change::StorageRoot:
			account.setStorageRoot(change.value);
			break;
		case Change::Balance:
			account.addBalance(0 - change.value);
			break;
		case Change::Nonce:
			account.setNonce(change.value);
			break;
		case Change::Create:
			m_cache.erase(change.address);
			break;
		case Change::Code:
			account.setCode(std::move(change.oldCode));
			break;
		case Change::Touch:
			account.untouch();
			m_unchangedCacheEntries.emplace_back(change.address);
			break;
		}
		m_changeLog.pop_back();
	}
}

void EosState::kill(Address _addr)
{
	if (auto a = account(_addr))
		a->kill();
	// If the account is not in the db, nothing to kill.
}


u256 EosState::getNonce(Address const& _addr) const
{
	if (auto a = account(_addr))
		return a->nonce();
	else
		return m_accountStartNonce;
}

u256 EosState::balance(Address const& _id) const
{
	if (auto a = account(_id))
		return a->balance();
	else
		return 0;
}

void EosState::subBalance(Address const& _addr, u256 const& _value)
{
	if (_value == 0)
		return;

	Account* a = account(_addr);
	if (!a || a->balance() < _value)
		// TODO: I expect this never happens.
		BOOST_THROW_EXCEPTION(NotEnoughCash());

	// Fall back to addBalance().
	addBalance(_addr, 0 - _value);
}

void EosState::incNonce(Address const& _addr)
{
	ilog("");

	if (Account* a = account(_addr))
	{
		auto oldNonce = a->nonce();
		a->incNonce();
		m_changeLog.emplace_back(_addr, oldNonce);
	}
	else
		// This is possible if a transaction has gas price 0.
		createAccount(_addr, Account(requireAccountStartNonce() + 1, 0));
}

size_t EosState::savepoint() const
{
	return m_changeLog.size();
}

bool EosState::addressHasCode(Address const& _id) const
{
	ilog("");

	int size;
	uint64_t n = ((uint64_t*)_id.data())[0];
	return get_code_size(n, size);

	if (auto a = account(_id))
		return a->codeHash() != EmptySHA3;
	else
		return false;
}

h256 EosState::codeHash(Address const& _a) const
{
	ilog("");
	return sha3(code(_a));
	if (Account const* a = account(_a))
		return a->codeHash();
	else
		return EmptySHA3;
}

u256 const& EosState::requireAccountStartNonce() const
{
	ilog("");

	if (m_accountStartNonce == Invalid256)
		BOOST_THROW_EXCEPTION(InvalidAccountStartNonceInState());
	return m_accountStartNonce;
}

bool EosState::addressInUse(Address const& _id) const
{
	uint64_t n = ((uint64_t*)_id.data())[0];
	int size = 0;
	bool ret;
	ret = get_code_size(n, size);
	ilog( "size ${n}\n", ("n", size) );
	return ret;
	//	return !!account(_id);
}

bool EosState::accountNonemptyAndExisting(Address const& _address) const
{
	ilog("");

	uint64_t n = ((uint64_t*)_address.data())[0];
	int size = 0;
	return get_code_size(n, size);
/*
	if (Account const* a = account(_address))
		return !a->isEmpty();
	else
		return false;
*/
}
#endif

#include <libdevcore/CommonJS.h>
vector<uint8_t> g_code;
bytes const& EosState::code(Address const& _addr) const
{
	ilog(_addr.hex());
	uint64_t n = ((uint64_t*)_addr.data())[0];
	if (get_code(n, g_code)) {
		ilog( "${n}", ("n", g_code.size()) );
		return g_code;
	}
	return NullBytes;

	Account const* a = account(_addr);
	if (!a || a->codeHash() == EmptySHA3)
		return NullBytes;

	if (a->code().empty())
	{
		// Load the code from the backend.
		Account* mutableAccount = const_cast<Account*>(a);
		mutableAccount->noteCode(m_db.lookup(a->codeHash()));
		CodeSizeCache::instance().store(a->codeHash(), a->code().size());
	}

	return a->code();

}


size_t EosState::codeSize(Address const& _a) const
{
	uint64_t n = ((uint64_t*)_a.data())[0];
	int size = 0;
	get_code_size(n, size);
	return size;
}

void EosState::setStorage(Address const& _contract, u256 const& _key, u256 const& _value)
{
	m_changeLog.emplace_back(_contract, _key, storage(_contract, _key));
//	m_cache[_contract].setStorage(_key, _value);
	uint64_t n = ((uint64_t*)_contract.data())[0];

	ilog( "${n1} : ${n2} : ${n3}", ("n1",_key.str())("n2",_value.str())("n3", n) );

	string key = _key.str();
	string value = _value.str();
	int ret = store_str( n, n, key.c_str(), key.size(), value.c_str(), value.size() );
	ilog( "${n1} : ${n2} : ${n3} ${n4} ${n5}", ("n1",_key.str())("n2",_value.str())("n3", n)("n4", ret)("n5", value.size()) );
}

u256 EosState::storage(Address const& _id, u256 const& _key) const
{
	ilog( "${n1} ${n2}", ("n1", _id.hex())("n2", _key.str()) );

	uint64_t n = ((uint64_t*)_id.data())[0];

	char data[256];
	memset(data, 0, sizeof(data));
	string key = _key.str();
	if (load_str(n, n, n, (char*)key.c_str(), key.size(), data, sizeof(data)) > 0) {
		u256 ret = u256(data);
		ilog( "${n1} ${n2}", ("n1", _key.str())("n2", data) );
		return ret;
	}
	return 0;

	if (Account const* a = account(_id))
	{
		auto mit = a->storageOverlay().find(_key);
		if (mit != a->storageOverlay().end())
			return mit->second;

		// Not in the storage cache - go to the DB.
		SecureTrieDB<h256, OverlayDB> memdb(const_cast<OverlayDB*>(&m_db), a->baseRoot());			// promise we won't change the overlay! :)
		string payload = memdb.at(_key);
		u256 ret = payload.size() ? RLP(payload).toInt<u256>() : 0;
		a->setStorageCache(_key, ret);
		return ret;
	}
	else
		return 0;

}


map<h256, pair<u256, u256>> EosState::storage(Address const& _id) const
{
	map<h256, pair<u256, u256>> ret;
	ilog("");

	if (Account const* a = account(_id))
	{
		// Pull out all values from trie storage.
		if (h256 root = a->baseRoot())
		{
			SecureTrieDB<h256, OverlayDB> memdb(const_cast<OverlayDB*>(&m_db), root);		// promise we won't alter the overlay! :)

			for (auto it = memdb.hashedBegin(); it != memdb.hashedEnd(); ++it)
			{
				h256 const hashedKey((*it).first);
				u256 const key = h256(it.key());
				u256 const value = RLP((*it).second).toInt<u256>();
				ret[hashedKey] = make_pair(key, value);
			}
		}

		// Then merge cached storage over the top.
		for (auto const& i : a->storageOverlay())
		{
			h256 const key = i.first;
			h256 const hashedKey = sha3(key);
			if (i.second)
				ret[hashedKey] = i;
			else
				ret.erase(hashedKey);
		}
	}
	return ret;
}

#if 0
void EosState::createAccount(Address const& _address, Account const&& _account)
{
	ilog("");

//	assert(!addressInUse(_address) && "Account already exists");
	m_cache[_address] = std::move(_account);
	m_nonExistingAccountsCache.erase(_address);
	m_changeLog.emplace_back(Change::Create, _address);
}
#endif
