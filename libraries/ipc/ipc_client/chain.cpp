/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */

extern "C" {

#warning FIXME: get_active_producers
uint32_t get_active_producers( uint64_t* producers, uint32_t buffer_size ) {
   return ipc_client::get().get_active_producers( producers, buffer_size );
}

}
