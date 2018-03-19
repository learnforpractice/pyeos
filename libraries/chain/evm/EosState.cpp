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

using namespace dev::eth;
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
	if (auto a = account(_id))
		return a->codeHash() != EmptySHA3;
	else
		return false;
}

bytes const& EosState::code(Address const& _addr) const
{
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

h256 EosState::codeHash(Address const& _a) const
{
	if (Account const* a = account(_a))
		return a->codeHash();
	else
		return EmptySHA3;
}

u256 const& EosState::requireAccountStartNonce() const
{
	if (m_accountStartNonce == Invalid256)
		BOOST_THROW_EXCEPTION(InvalidAccountStartNonceInState());
	return m_accountStartNonce;
}
