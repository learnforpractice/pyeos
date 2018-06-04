#include <eosiolib/types.h>
#include <eosiolib/privileged.h>
void set_resource_limits( account_name account, int64_t ram_bytes, int64_t net_weight, int64_t cpu_weight ) {

}


int64_t set_proposed_producers( char *producer_data, uint32_t producer_data_size ) {
   return 0;
}

bool is_privileged( account_name account )  {
   return 0;
}

void set_privileged( account_name account, bool is_priv ) {

}

void set_blockchain_parameters_packed(char* data, uint32_t datalen) {

}

uint32_t get_blockchain_parameters_packed(char* data, uint32_t datalen) {
   return 0;
}

void activate_feature( int64_t f ) {

}


