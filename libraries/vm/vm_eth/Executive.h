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


#include <Common.h>
#include <VMFace.h>

#include <functional>

#include <eosiolib/types.hpp>
#include "Transaction.h"


namespace dev
{

namespace eth
{

class State;
class Block;
class BlockChain;
class ExtVM;
class SealEngineFace;
struct Manifest;

class StandardTrace
{
public:
    struct DebugOptions
    {
        bool disableStorage = false;
        bool disableMemory = false;
        bool disableStack = false;
        bool fullStorage = false;
    };

    StandardTrace();
    void operator()(uint64_t _steps, uint64_t _PC, Instruction _inst, bigint _newMemSize,
        bigint _gasCost, bigint _gas, VMFace const* _vm, ExtVMFace const* _extVM);

    void setShowMnemonics() { m_showMnemonics = true; }
    void setOptions(DebugOptions _options) { m_options = _options; }

    std::string styledJson() const;
    std::string multilineTrace() const;

    OnOpFunc onOp()
    {
        return [=](uint64_t _steps, uint64_t _PC, Instruction _inst, bigint _newMemSize,
                   bigint _gasCost, bigint _gas, VMFace const* _vm, ExtVMFace const* _extVM) {
            (*this)(_steps, _PC, _inst, _newMemSize, _gasCost, _gas, _vm, _extVM);
        };
    }

private:
    bool m_showMnemonics = false;
    std::vector<Instruction> m_lastInst;
    DebugOptions m_options;
};

/**
 * @brief Message-call/contract-creation executor; useful for executing transactions.
 *
 * Two ways of using this class - either as a transaction executive or a CALL/CREATE executive.
 *
 * In the first use, after construction, begin with initialize(), then execute() and end with finalize(). Call go()
 * after execute() only if it returns false.
 *
 * In the second use, after construction, begin with call() or create() and end with
 * accrueSubState(). Call go() after call()/create() only if it returns false.
 *
 * Example:
 * @code
 * Executive e(state, blockchain, 0);
 * e.initialize(transaction);
 * if (!e.execute())
 *    e.go();
 * e.finalize();
 * @endcode
 */
class Executive
{
public:
    /// Simple constructor; executive will operate on given state, with the given environment info.
    Executive(State& _s, EnvInfo const& _envInfo, unsigned _level = 0): m_s(_s), m_envInfo(_envInfo), m_depth(_level) {}

    /** Easiest constructor.
     * Creates executive to operate on the state of end of the given block, populating environment
     * info from given Block and the LastHashes portion from the BlockChain.
     */
    Executive(unsigned _level = 0);


    /** Previous-state constructor.
     * Creates executive to operate on the state of a particular transaction in the given block,
     * populating environment info from the given Block and the LastHashes portion from the BlockChain.
     * State is assigned the resultant value, but otherwise unused.
     */
    Executive(State& io_s, unsigned _txIndex, unsigned _level = 0);

    Executive(Executive const&) = delete;
    void operator=(Executive) = delete;

    void initialize(Transaction const& _transaction);
    /// Finalise a transaction previously set up with initialize().
    /// @warning Only valid after initialize() and execute(), and possibly go().
    /// @returns true if the outermost execution halted normally, false if exceptionally halted.
    bool finalize();
    /// Begins execution of a transaction. You must call finalize() following this.
    /// @returns true if the transaction is done, false if go() must be called.
    bool execute();
    /// @returns the transaction from initialize().
    /// @warning Only valid after initialize().
    Transaction const& t() const { return m_t; }

    /// @returns total gas used in the transaction/operation.
    /// @warning Only valid after finalise().
    u256 gasUsed() const;

    owning_bytes_ref takeOutput() { return std::move(m_output); }

    /// Set up the executive for evaluating a bare CREATE (contract-creation) operation.
    /// @returns false iff go() must be called (and thus a VM execution in required).
    bool create(Address const& _txSender, u256 const& _endowment, bytesConstRef _code, Address const& _originAddress);
    /// @returns false iff go() must be called (and thus a VM execution in required).
    bool createOpcode(Address const& _sender, u256 const& _endowment, bytesConstRef _code, Address const& _originAddress);
    /// @returns false iff go() must be called (and thus a VM execution in required).
    bool create2Opcode(Address const& _sender, u256 const& _endowment, bytesConstRef _code, Address const& _originAddress, u256 const& _salt);
    /// Set up the executive for evaluating a bare CALL (message call) operation.
    /// @returns false iff go() must be called (and thus a VM execution in required).
    bool call(Address const& _receiveAddress, Address const& _txSender, u256 const& _txValue, bytesConstRef _txData);
    bool call(CallParameters const& _cp, Address const& _origin);
    /// Finalise an operation through accruing the substate into the parent context.
    void accrueSubState(SubState& _parentContext);

    /// Executes (or continues execution of) the VM.
    /// @returns false iff go() must be called again to finish the transaction.
    bool go(OnOpFunc const& _onOp = OnOpFunc());

    /// Operation function for providing a simple trace of the VM execution.
    OnOpFunc simpleTrace();

    /// @returns gas remaining after the transaction/operation. Valid after the transaction has been executed.
    u256 gas() const { return m_gas; }

    /// @returns the new address for the created contract in the CREATE operation.
    Address newAddress() const { return m_newAddress; }

    /// @returns The exception that has happened during the execution if any.
    TransactionException getException() const noexcept { return m_excepted; }

    /// Revert all changes made to the state by this execution.
    void revert();

private:
    /// @returns false iff go() must be called (and thus a VM execution in required).
    bool executeCreate(Address const& _txSender, u256 const& _endowment, bytesConstRef _code, Address const& _originAddress);

    State& m_s;                     ///< The state to which this operation/transaction is applied.
    // TODO: consider changign to EnvInfo const& to avoid LastHashes copy at every CALL/CREATE
    EnvInfo m_envInfo;              ///< Information on the runtime environment.
    std::shared_ptr<ExtVM> m_ext;      ///< The VM externality object for the VM execution or null if no VM is required. shared_ptr used only to allow ExtVM forward reference. This field does *NOT* survive this object.
    owning_bytes_ref m_output;         ///< Execution output.

    TransactionException m_excepted = TransactionException::None; ///< Details if the VM's execution resulted in an exception.

    unsigned m_depth = 0;           ///< The context's call-depth.
    int64_t m_baseGasRequired;         ///< The base amount of gas requried for executing this transaction.
    u256 m_gas = 0;                 ///< The gas for EVM code execution. Initial amount before go() execution, final amount after go() execution.
    u256 m_refunded = 0;            ///< The amount of gas refunded.

    Transaction m_t;             ///< The original transaction. Set by setup().

    u256 m_gasCost;

    bool m_isCreation = false;
    Address m_newAddress;
    size_t m_savepoint = 0;

};

}
}
