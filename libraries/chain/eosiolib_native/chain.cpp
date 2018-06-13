/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */



uint32_t get_active_producers( account_name* producers, uint32_t buffer_size ) {
   auto active_producers = ctx().get_active_producers();

   size_t len = active_producers.size();
   auto s = len * sizeof(chain::account_name);
   if( buffer_size == 0 ) return s;

   auto copy_size = std::min( (size_t)buffer_size, s );
   memcpy( producers, active_producers.data(), copy_size );

   return copy_size;
}


