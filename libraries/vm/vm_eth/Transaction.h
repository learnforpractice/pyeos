#pragma once

#include <Address.h>
#include <Exceptions.h>

namespace dev
{

namespace eth
{
   enum class TransactionException
   {
      None = 0,
      Unknown,
      BadRLP,
      InvalidFormat,
      OutOfGasIntrinsic,      ///< Too little gas to pay for the base transaction cost.
      InvalidSignature,
      InvalidNonce,
      NotEnoughCash,
      OutOfGasBase,        ///< Too little gas to pay for the base transaction cost.
      BlockGasLimitReached,
      BadInstruction,
      BadJumpDestination,
      OutOfGas,            ///< Ran out of gas executing code of the transaction.
      OutOfStack,          ///< Ran out of stack executing code of the transaction.
      StackUnderflow,
      RevertInstruction,
      InvalidZeroSignatureFormat,
      AddressAlreadyUsed
   };

   struct Transaction {
      Address m_receiveAddress;
      Address m_sender;
      bytes m_data;
      u256 m_value;
      bool m_creation;
      Address sender() {
         return m_sender;
      }
      Address receiveAddress() {
         return m_receiveAddress;
      }
      bool isCreation() {
         return m_creation;
      }
      u256 value() {
         return m_value;
      }
      bytes const& data() {
         return m_data;
      }
   };

   static TransactionException toTransactionException(Exception const& _e)
   {
      // Basic Transaction exceptions
#if 0
      if (!!dynamic_cast<RLPException const*>(&_e))
         return TransactionException::BadRLP;
      if (!!dynamic_cast<OutOfGasIntrinsic const*>(&_e))
         return TransactionException::OutOfGasIntrinsic;
      if (!!dynamic_cast<InvalidSignature const*>(&_e))
         return TransactionException::InvalidSignature;
      // Executive exceptions
      if (!!dynamic_cast<OutOfGasBase const*>(&_e))
         return TransactionException::OutOfGasBase;
      if (!!dynamic_cast<InvalidNonce const*>(&_e))
         return TransactionException::InvalidNonce;
      if (!!dynamic_cast<NotEnoughCash const*>(&_e))
         return TransactionException::NotEnoughCash;
      if (!!dynamic_cast<BlockGasLimitReached const*>(&_e))
         return TransactionException::BlockGasLimitReached;
      if (!!dynamic_cast<AddressAlreadyUsed const*>(&_e))
         return TransactionException::AddressAlreadyUsed;
      // VM execution exceptions
      if (!!dynamic_cast<BadInstruction const*>(&_e))
         return TransactionException::BadInstruction;
      if (!!dynamic_cast<BadJumpDestination const*>(&_e))
         return TransactionException::BadJumpDestination;
      if (!!dynamic_cast<OutOfGas const*>(&_e))
         return TransactionException::OutOfGas;
      if (!!dynamic_cast<OutOfStack const*>(&_e))
         return TransactionException::OutOfStack;
      if (!!dynamic_cast<StackUnderflow const*>(&_e))
         return TransactionException::StackUnderflow;
#endif
      return TransactionException::Unknown;
   }

}
}
