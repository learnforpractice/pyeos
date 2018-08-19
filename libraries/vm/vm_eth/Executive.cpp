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

#include "Executive.h"


#include "ExtVM.h"
#include "State.h"
#include <LegacyVM.h>
#include <VMFactory.h>

#include <boost/timer.hpp>

#include <numeric>
#include "SealEngine.h"

using namespace std;
using namespace dev;
using namespace dev::eth;


Executive::Executive(State& io_s, unsigned _txIndex, unsigned _level):
    m_s(io_s),
    m_depth(_level)
{
}

void Executive::accrueSubState(SubState& _parentContext)
{
    if (m_ext)
        _parentContext += m_ext->sub;
}

void Executive::initialize(Transaction const& _transaction)
{
   m_t = _transaction;
}

bool Executive::execute()
{
    if (m_t.isCreation())
        return create(m_t.sender(), m_t.value(), &m_t.data(), m_t.sender());
    else
        return call(m_t.receiveAddress(), m_t.sender(), m_t.value(), bytesConstRef(&m_t.data()));
}

bool Executive::call(Address const& _receiveAddress, Address const& _senderAddress, u256 const& _value, bytesConstRef _data)
{
    CallParameters params{_senderAddress, _receiveAddress, _receiveAddress, _value, _value, 0, _data, {}};
    return call(params, _senderAddress);
}

bool Executive::call(CallParameters const& _p, Address const& _origin)
{
    if (SealEngine::get().isPrecompiled(_p.codeAddress, m_envInfo.number()))
    {
         bytes output;
         bool success;
         tie(success, output) = SealEngine::get().executePrecompiled(_p.codeAddress, _p.data, m_envInfo.number());
         size_t outputSize = output.size();
         m_output = owning_bytes_ref{std::move(output), 0, outputSize};
         if (!success)
         {
             m_gas = 0;
             m_excepted = TransactionException::OutOfGas;
             return true;  // true means no need to run go().
         }
    }
    else
    {
        m_gas = _p.gas;
        if (m_s.addressHasCode(_p.codeAddress))
        {
            bytes const& c = m_s.code(_p.codeAddress);
            h256 codeHash = m_s.codeHash(_p.codeAddress);
            m_ext = make_shared<ExtVM>(m_s, m_envInfo, _p.receiveAddress,
                _p.senderAddress, _origin, _p.apparentValue, 0, _p.data, &c, codeHash,
                m_depth, false, _p.staticCall);
        }
    }

    // Transfer ether.
    m_s.transferBalance(_p.senderAddress, _p.receiveAddress, _p.valueTransfer);
    return !m_ext;
}

bool Executive::create(Address const& _txSender, u256 const& _endowment, bytesConstRef _init, Address const& _origin)
{
    // Contract creation by an external account is the same as CREATE opcode
    return createOpcode(_txSender, _endowment, _init, _origin);
}

bool Executive::createOpcode(Address const& _sender, u256 const& _endowment, bytesConstRef _init, Address const& _origin)
{
    u256 nonce = m_s.getNonce(_sender);
    m_newAddress = Address(0);//right160(sha3(rlpList(_sender, nonce)));
    return executeCreate(_sender, _endowment, _init, _origin);
}

bool Executive::create2Opcode(Address const& _sender, u256 const& _endowment, bytesConstRef _init, Address const& _origin, u256 const& _salt)
{
    m_newAddress = Address(0);//right160(sha3(bytes{0xff} +_sender.asBytes() + toBigEndian(_salt) + sha3(_init)));
    return executeCreate(_sender, _endowment, _init, _origin);
}

bool Executive::executeCreate(Address const& _sender, u256 const& _endowment, bytesConstRef _init, Address const& _origin)
{
    m_isCreation = true;

    // We can allow for the reverted state (i.e. that with which m_ext is constructed) to contain the m_orig.address, since
    // we delete it explicitly if we decide we need to revert.

    bool accountAlreadyExist = (m_s.addressHasCode(m_newAddress) || m_s.getNonce(m_newAddress) > 0);
    if (accountAlreadyExist)
    {
//        LOG(m_detailsLogger) << "Address already used: " << m_newAddress;
        m_gas = 0;
        m_excepted = TransactionException::AddressAlreadyUsed;
        revert();
        m_ext = {}; // cancel the _init execution if there are any scheduled.
        return !m_ext;
    }

    // Transfer ether before deploying the code. This will also create new
    // account if it does not exist yet.
    m_s.transferBalance(_sender, m_newAddress, _endowment);

    // Schedule _init execution if not empty.
    if (!_init.empty())
        m_ext = make_shared<ExtVM>(m_s, m_envInfo, m_newAddress, _sender, _origin,
              _endowment, 0, bytesConstRef(), _init, u256(0), m_depth, true, false);
//FIXME sha3            _endowment, _gasPrice, bytesConstRef(), _init, sha3(_init), m_depth, true, false);



    return !m_ext;
}

bool Executive::go(bytes& output, OnOpFunc const& _onOp)
{
   if (!m_ext)
   {
      return true;
   }
   try
   {
      // Create VM instance. Force Interpreter if tracing requested.
      auto vm = VMFactory::create();
      if (m_isCreation)
      {
          m_s.clearStorage(m_ext->myAddress);
          auto out = vm->exec(m_gas, *m_ext, _onOp);
          output = out.toVector(); // copy output to execution result
          m_s.setCode(m_ext->myAddress, out.toVector());
      }
      else
          m_output = vm->exec(m_gas, *m_ext, _onOp);
   }
   catch (RevertInstruction& _e)
   {
      revert();
      m_output = _e.output();
      m_excepted = TransactionException::RevertInstruction;
   }
   catch (VMException const& _e)
   {
//      LOG(m_detailsLogger) << "Safe VM Exception. " << diagnostic_information(_e);
      m_gas = 0;
      m_excepted = toTransactionException(_e);
      revert();
   }
   catch (InternalVMError const& _e)
   {
   //            cwarn << "Internal VM Error (" << *boost::get_error_info<errinfo_evmcStatusCode>(_e) << ")\n" << diagnostic_information(_e);
      revert();
      throw;
   }
   catch (Exception const& _e)
   {
      // TODO: AUDIT: check that this can never reasonably happen. Consider what to do if it does.
      //cwarn << "Unexpected exception in VM. There may be a bug in this implementation. " << diagnostic_information(_e);
      exit(1);
      // Another solution would be to reject this transaction, but that also
      // has drawbacks. Essentially, the amount of ram has to be increased here.
   }
   catch (std::exception const& _e)
   {
      // TODO: AUDIT: check that this can never reasonably happen. Consider what to do if it does.
      //cwarn << "Unexpected std::exception in VM. Not enough RAM? " << _e.what();
      exit(1);
      // Another solution would be to reject this transaction, but that also
      // has drawbacks. Essentially, the amount of ram has to be increased here.
   }

   if (m_output) {
     // Copy full output:
      output = m_output.toVector();
   }
    return true;
}

bool Executive::finalize()
{
    // Suicides...
    if (m_ext)
        for (auto a: m_ext->sub.suicides)
            m_s.kill(a);

    return (m_excepted == TransactionException::None);
}

void Executive::revert()
{
    if (m_ext)
        m_ext->sub.clear();

    // Set result address to the null one.
    m_newAddress = {};
}
