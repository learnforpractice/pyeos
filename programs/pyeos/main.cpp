#include <stdlib.h>
#include <string.h>

extern "C" {
   int eos_main(int argc, char** argv);
   int start_client(){
      return 0;
   };

   int start_server(){
      return 0;
   };
}

int main(int argc, char** argv) {
   bool client = false;
   bool server = false;

   for (int i=0; i<argc; i++) {
      if (strcmp(argv[i], "--client") == 0) {
         client = 1;
      }
      if (strcmp(argv[i], "--server") == 0) {
         server = 1;
      }
   }

   if (client) {
      start_client();
   } else {
      if (server) {
         start_server();
      }
      return eos_main(argc, argv);
   }
}


