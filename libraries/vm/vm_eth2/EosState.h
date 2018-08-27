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
#include <libdevcore/Exceptions.h>
#include <libevm/ExtVMFace.h>

namespace dev
{

namespace test { class ImportTest; class StateLoader; }

namespace eth
{

// Import-specific errinfos
using errinfo_uncleIndex = boost::error_info<struct tag_uncleIndex, unsigned>;
using errinfo_currentNumber = boost::error_info<struct tag_currentNumber, u256>;
using errinfo_uncleNumber = boost::error_info<struct tag_uncleNumber, u256>;
using errinfo_unclesExcluded = boost::error_info<struct tag_unclesExcluded, h256Hash>;
using errinfo_block = boost::error_info<struct tag_block, bytes>;
using errinfo_now = boost::error_info<struct tag_now, unsigned>;

using errinfo_transactionIndex = boost::error_info<struct tag_transactionIndex, unsigned>;

using errinfo_vmtrace = boost::error_info<struct tag_vmtrace, std::string>;
using errinfo_receipts = boost::error_info<struct tag_receipts, std::vector<bytes>>;
using errinfo_transaction = boost::error_info<struct tag_transaction, bytes>;
using errinfo_phase = boost::error_info<struct tag_phase, unsigned>;
using errinfo_required_LogBloom = boost::error_info<struct tag_required_LogBloom, LogBloom>;
using errinfo_got_LogBloom = boost::error_info<struct tag_get_LogBloom, LogBloom>;
using LogBloomRequirementError = boost::tuple<errinfo_required_LogBloom, errinfo_got_LogBloom>;

class BlockChain;
class EosState;
class TransactionQueue;
struct VerifiedBlockRef;

enum class BaseState
{
    PreExisting,
    Empty
};

enum class Permanence
{
    Reverted,
    Committed,
    Uncommitted   ///< Uncommitted state for change log readings in tests.
};

DEV_SIMPLE_EXCEPTION(InvalidAccountStartNonceInState);
DEV_SIMPLE_EXCEPTION(IncorrectAccountStartNonceInState);

class SealEngineFace;
class Executive;

/// An atomic state changelog entry.
struct Change
{
    enum Kind: int
    {
        /// Account balance changed. Change::value contains the amount the
        /// balance was increased by.
        Balance,

        /// Account storage was modified. Change::key contains the storage key,
        /// Change::value the storage value.
        Storage,

        /// Account storage root was modified.  Change::value contains the old
        /// account storage root.
        StorageRoot,

        /// Account nonce was changed.
        Nonce,

        /// Account was created (it was not existing before).
        Create,

        /// New code was added to an account (by "create" message execution).
        Code,

        /// Account was touched for the first time.
        Touch
    };

    Kind kind;        ///< The kind of the change.
    Address address;  ///< Changed account address.
    u256 value;       ///< Change value, e.g. balance, storage and nonce.
    u256 key;         ///< Storage key. Last because used only in one case.
    bytes oldCode;    ///< Code overwritten by CREATE, empty except in case of address collision.

    /// Helper constructor to make change log update more readable.
    Change(Kind _kind, Address const& _addr, u256 const& _value = 0):
            kind(_kind), address(_addr), value(_value)
    {
        assert(_kind != Code); // For this the special constructor needs to be used.
    }

    /// Helper constructor especially for storage change log.
    Change(Address const& _addr, u256 const& _key, u256 const& _value):
            kind(Storage), address(_addr), value(_value), key(_key)
    {}

    /// Helper constructor for nonce change log.
    Change(Address const& _addr, u256 const& _value):
            kind(Nonce), address(_addr), value(_value)
    {}

    /// Helper constructor especially for new code change log.
    Change(Address const& _addr, bytes const& _oldCode):
            kind(Code), address(_addr), oldCode(_oldCode)
    {}
};

using ChangeLog = std::vector<Change>;

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
class EosState
{
    friend class ExtVM;
    friend class dev::test::ImportTest;
    friend class dev::test::StateLoader;
    friend class BlockChain;

public:
    enum class CommitBehaviour
    {
        KeepEmptyAccounts,
        RemoveEmptyAccounts
    };

    /// Basic state object from database.
    /// Use the default when you already have a database and you just want to make a EosState object
    /// which uses it. If you have no preexisting database then set BaseState to something other
    /// than BaseState::PreExisting in order to prepopulate the Trie.
    EosState(){};

    /// Copy state object.
    EosState(EosState const& _s);

    /// Copy state object.
    EosState& operator=(EosState const& _s);


    /// Execute a given transaction.
    /// This will change the state accordingly.
//    std::pair<ExecutionResult, TransactionReceipt> execute(EnvInfo const& _envInfo, SealEngineFace const& _sealEngine, Transaction const& _t, Permanence _p = Permanence::Committed, OnOpFunc const& _onOp = OnOpFunc());


    /// Check if the address is in use.
    bool addressInUse(Address const& _address) const;

    /// Check if the address contains executable code.
    bool addressHasCode(Address const& _address) const;

    /// Get an account's balance.
    /// @returns 0 if the address has never been used.
    u256 balance(Address const& _id) const;

    /**
     * @brief Transfers "the balance @a _value between two accounts.
     * @param _from Account from which @a _value will be deducted.
     * @param _to Account to which @a _value will be added.
     * @param _value Amount to be transferred.
     */
    void transferBalance(Address const& _from, Address const& _to, u256 const& _value);

    /// Get the value of a storage position of an account.
    /// @returns 0 if no account exists at that address.
    u256 storage(Address const& _contract, u256 const& _memory) const;

    /// Set the value of a storage position of an account.
    void setStorage(Address const& _contract, u256 const& _location, u256 const& _value);

    /// Clear the storage root hash of an account to the hash of the empty trie.
    void clearStorage(Address const& _contract){};

    /// Sets the code of the account. Must only be called during / after contract creation.
    void setCode(Address const& _address, bytes&& _code);

    /// Delete an account (used for processing suicides).
    void kill(Address _a){};

    /// Get the storage of an account.
    /// @note This is expensive. Don't use it unless you need to.
    /// @returns map of hashed keys to key-value pairs or empty map if no account exists at that address.
    std::map<h256, std::pair<u256, u256>> storage(Address const& _contract) const;

    /// Get the code of an account.
    /// @returns bytes() if no account exists at that address.
    /// @warning The reference to the code is only valid until the access to
    ///          other account. Do not keep it.
    bytes const& code(Address const& _addr) const;

    /// Get the code hash of an account.
    /// @returns EmptySHA3 if no account exists at that address or if there is no code associated with the address.
    h256 codeHash(Address const& _contract) const;

    /// Get the byte-size of the code of an account.
    /// @returns code(_contract).size(), but utilizes CodeSizeHash.
    size_t codeSize(Address const& _contract) const;

protected:

    friend std::ostream& operator<<(std::ostream& _out, EosState const& _s);

};

std::ostream& operator<<(std::ostream& _out, EosState const& _s);


}
}

