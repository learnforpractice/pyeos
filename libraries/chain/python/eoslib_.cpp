#include "eoslib_.hpp"
extern "C" {
   int read_action_(char* memory, size_t size) {
      return read_action(memory, size);
   }
}

