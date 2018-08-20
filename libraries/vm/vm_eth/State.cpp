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
   assert(0);
   return false;
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

u256 State::getNonce(Address const& _addr) const
{

}

u256 State::storage(Address const& _id, u256 const& _key) const
{
   uint64_t code = _id;
   assert(0);
   return 0;
}

void State::setStorage(Address const& _contract, u256 const& _key, u256 const& _value)
{
   uint64_t code = _contract;
   assert(0);
}

void State::clearStorage(Address const& _contract)
{
   assert(0);
}

h256 State::storageRoot(Address const& _id) const
{
   assert(0);
}

bytes const& State::code(Address const& _addr) const
{
   assert(0);
   return bytes();
}

void State::setCode(Address const& _address, bytes&& _code)
{

}

h256 State::codeHash(Address const& _a) const
{
   return h256(0);//EmptySHA3;
}

size_t State::codeSize(Address const& _a) const
{
   return 0;
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
