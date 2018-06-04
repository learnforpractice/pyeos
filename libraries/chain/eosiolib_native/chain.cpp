/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */



uint32_t get_active_producers( account_name* producers, uint32_t datalen ) {
   array_ptr<chain::account_name> ptr(producers);
   return producer_api(ctx(), false).get_active_producers( ptr, datalen);
}
