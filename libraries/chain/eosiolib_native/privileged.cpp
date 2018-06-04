
void set_resource_limits( account_name account, int64_t ram_bytes, int64_t net_weight, int64_t cpu_weight ) {
   privileged_api(ctx()).set_resource_limits(account, ram_bytes, net_weight, cpu_weight);
}


int64_t set_proposed_producers( char *producer_data, uint32_t producer_data_size ) {
   return privileged_api(ctx()).set_proposed_producers(array_ptr<char>(producer_data), producer_data_size);
}

bool is_privileged( account_name account )  {
   return privileged_api(ctx()).is_privileged(account);
}

void set_privileged( account_name account, bool is_priv ) {
   privileged_api(ctx()).set_privileged( account, is_priv );
}

void set_blockchain_parameters_packed(char* data, uint32_t datalen) {
   privileged_api(ctx()).set_blockchain_parameters_packed( array_ptr<char>(data), datalen );
}

uint32_t get_blockchain_parameters_packed(char* data, uint32_t datalen) {
   return privileged_api(ctx()).get_blockchain_parameters_packed(array_ptr<char>(data), datalen);
}

void activate_feature( int64_t f ) {
   return privileged_api(ctx()).activate_feature(f);
}


