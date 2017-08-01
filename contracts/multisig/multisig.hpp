#pragma once


namespace multisig {


   /**
    * Proposes a new transaction and stores the transaction data in `proposer/multisig/...` so
    * it will require the scope of 'proposer'
    *
    */
   struct propose {
      AccountName           proposer;   ///< the account name proposing the transaction
      Time                  valid_on;   ///< the earliest the tranasction can be applied
      Time                  expire;     ///< the time the proposal will expire
      transaction_id_type   id;
      vector<char>          packed_transaction;
   };

   /**
    * This message will cache the auth used on the approve message so
    * that it can be applied.
    *
    * Requires the scope of proposer and that id is a previously proposed transaction
    */
   struct approve {
      AccountName         proposer;
      transaction_id_type id;
   };

   /**
    * For each auth specified on this message it will clear it.
    *
    * Requires the scope of proposer
    */
   struct reject {
      AccountName         proposer;
      transaction_id_type id;
   };

   /**
    * This message can be delivered anytime between valid_on and expire for
    * the proposed transaction. When it is delivered it should have the necessary
    * scope to execute the proposed transaction synchronously.
    *
    * This message will be rejected if the contract doesn't have all of the
    * necessary authorities cached and loaded.
    */
   struct exec {
      AccountName         proposer;
      transaction_id_type id;
   };


   /**
    *  The purpose of this is to store an always incrementing count that
    *  acts as a shorter unique identifier than the transaction id. It gets
    *  incremented every time a new @ref propose message is used wthin the
    *  current scope.
    */
   struct sequence_number {
      static const uint64_t key = N(seq)
      uint64_t              next = 0;
   };


   /**
    *  This database table is an i128i128 table that uses the transaction ID 
    *  as the key. It is used to lookup the sequence number and packed transaction.
    */
   struct proposed_transaction_header {
      transaction_id_type   id;
      uint64_t              sequence;
   };

   /**
    *  This database table is an i128i128 table that uses the transaction ID 
    *  as the key. It is used to lookup the sequence number and packed transaction.
    *
    *  Normally we only need to lookup the sequence and there is no need to fetch the
    *  packed transaction.
    */
   struct proposed_transaction : proposed_transaction_header {
      vector<char>          packed_transaction;
   };

   struct approved_transaction {
      AccountName         account;
      PermissionName      approval;
      uint64_t            sequence;
   };


}
