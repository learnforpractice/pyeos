#include <stdlib.h>
#include <string.h>

extern "C" {
   int eos_main(int argc, char** argv);
   void rpc_register_cpp_apply_call();
   void cpp_init_eosapi();
   void set_client_mode(int mode);
   int start_client();
   int start_server();
}

int main(int argc, char** argv) {
   rpc_register_cpp_apply_call();
   cpp_init_eosapi();
   int client = 0;

   for (int i=0; i<argc; i++) {
      if (strcmp(argv[i], "--client") == 0) {
         client = 1;
         break;
      }
   }

   if (client) {
      start_client();
   } else {
      start_server();
      return eos_main(argc, argv);
   }
}


