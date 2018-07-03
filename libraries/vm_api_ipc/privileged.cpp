
extern "C" {

void set_resource_limits( uint64_t account, int64_t ram_bytes, int64_t net_weight, int64_t cpu_weight ) {
   return;
}

void get_resource_limits( uint64_t account, int64_t* ram_bytes, int64_t* net_weight, int64_t* cpu_weight ) {
   return;
}

int64_t set_proposed_producers( char *packed_producer_schedule, uint32_t datalen ) {
   return 0;
}

bool is_privileged( uint64_t n )  {
   return false;
}

void set_privileged( uint64_t n, bool is_priv ) {

}

void set_blockchain_parameters_packed(char* packed_blockchain_parameters, uint32_t datalen) {

}

uint32_t get_blockchain_parameters_packed(char* packed_blockchain_parameters, uint32_t buffer_size) {
   return 0;
}

void activate_feature( int64_t f ) {
}

}


