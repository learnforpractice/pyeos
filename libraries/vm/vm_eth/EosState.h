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

#pragma once

#include <array>
#include <unordered_map>
#include <libdevcore/Common.h>
#include <libdevcore/RLP.h>
#include <libdevcore/TrieDB.h>
#include <libdevcore/OverlayDB.h>
#include <libethcore/Exceptions.h>
#include <libethcore/BlockHeader.h>
#include <libethereum/CodeSizeCache.h>
#include <libevm/ExtVMFace.h>
#include <libethereum/Account.h>
#include <libethereum/Transaction.h>
#include <libethereum/TransactionReceipt.h>
#include <libethereum/GasPricer.h>

#define ETH_FATDB 1
#include <libethereum/State.h>

#include <leveldb/db.h>

using namespace std;
using namespace dev;

namespace dev
{

namespace test { class ImportTest; class StateLoader; }

namespace eth
{

/**
 * Model of an Ethereum state, essentially a facade for the trie.
 *
 * Allows you to query the state of accounts as well as creating and modifying
 * accounts. It has built-in caching for various aspects of the state.
 *
 * # EosState Changelog
 *
 * Any atomic change to any account is registered and appended in the changelog.
 * In case some changes must be reverted, the changes are popped from the
 * changelog and undone. For possible atomic changes list @see Change::Kind.
 * The changelog is managed by savepoint(), rollback() and commit() methods.
 */
class EosState : public dev::eth::State
{
public:
	EosState():State(0){}
	virtual ~EosState(){}
//	EosState(leveldb::DB& db):State(Invalid256, OverlayDB(&db), BaseState::Empty){}
#if 0
	virtual void setCode(Address const& _address, bytes&& _code);
	virtual void clearStorage(Address const& _contract);
	virtual void addBalance(Address const& _id, u256 const& _amount);

	virtual void rollback(size_t _savepoint);

	virtual void kill(Address _a);

	virtual u256 getNonce(Address const& _addr) const;

	virtual u256 balance(Address const& _id) const;

	virtual void subBalance(Address const& _addr, u256 const& _value);

	virtual void incNonce(Address const& _id);

	virtual size_t savepoint() const;

	virtual bool addressHasCode(Address const& _address) const;


	virtual h256 codeHash(Address const& _contract) const;

	virtual void transferBalance(Address const& _from, Address const& _to, u256 const& _value) { subBalance(_from, _value); addBalance(_to, _value); }

	virtual u256 const& requireAccountStartNonce() const;

	virtual bool addressInUse(Address const& _address) const;

	virtual bool accountNonemptyAndExisting(Address const& _address) const;
#endif

	virtual bytes const& code(Address const& _addr) const;


	virtual size_t codeSize(Address const& _contract) const;

	virtual void setStorage(Address const& _contract, u256 const& _location, u256 const& _value);

	virtual u256 storage(Address const& _contract, u256 const& _memory) const;

	virtual std::map<h256, std::pair<u256, u256>> storage(Address const& _contract) const;


#if 0
	virtual void createAccount(Address const& _address, Account const&& _account);
#endif
};

std::ostream& operator<<(std::ostream& _out, EosState const& _s);

EosState& createIntermediateState(EosState& o_s, Block const& _block, unsigned _txIndex, BlockChain const& _bc);

template <class DB>
AddressHash commit(AccountMap const& _cache, SecureTrieDB<Address, DB>& _state);

}
}

