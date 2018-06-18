/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */

static void abort() {
   edump(("abort() called"));
   FC_ASSERT( false, "abort() called");
}

static void  eosio_assert( uint32_t test, const char* msg ) {
   if( BOOST_UNLIKELY( !test ) ) {
      std::string message( msg );
      edump((message));
      EOS_THROW( eosio_assert_message_exception, "assertion failure with message: ${s}", ("s",message) );
   }
}

static void  eosio_assert_message( uint32_t test, const char* msg, uint32_t msg_len ) {
   if( BOOST_UNLIKELY( !test ) ) {
      std::string message( msg, msg_len );
      edump((message));
      EOS_THROW( eosio_assert_message_exception, "assertion failure with message: ${s}", ("s",message) );
   }
}

static void  eosio_assert_code( uint32_t test, uint64_t error_code ) {
   if( BOOST_UNLIKELY( !test ) ) {
      edump((error_code));
      EOS_THROW( eosio_assert_code_exception,
                 "assertion failure with error code: ${error_code}", ("error_code", error_code) );
   }
}

static void  eosio_exit( int32_t code ) {
   throw wasm_exit{code};
}

static uint64_t  current_time() {
   return static_cast<uint64_t>( ctx().control.pending_block_time().time_since_epoch().count() );
}

static uint32_t  now() {
   return (uint32_t)( current_time() / 1000000 );
}

static void checktime() {
   ctx().trx_context.checktime();
}

static void check_context_free(bool context_free) {
   if( ctx().context_free )
      FC_ASSERT( context_free, "only context free api's can be used in this context" );
   ctx().used_context_free_api |= !context_free;
}

static bool contracts_console() {
   return ctx().control.contracts_console();
}
