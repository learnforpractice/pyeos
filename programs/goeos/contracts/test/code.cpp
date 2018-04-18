#include <eoslib/db.h>
#include <eoslib/types.hpp>
#include <eoslib/message.h>
#include <eoslib/print.hpp>

extern "C"{
   int pythonLoad(uint64_t name, const char* codeptr, int codesize);
   int pythonCall(uint64_t name, uint64_t func, void* argsptr, int argssize);
}

int strlen(const char *code){
   int length = 0;
   while(code[length++]);
   return length;
}

using namespace eosio;

void read_length(char *raw_array,int& length,int& length_size){
   uint64_t v = 0; char b = 0; uint8_t by = 0;
   int index = 0;
   do {
       b = raw_array[index++];
       v |= uint32_t(uint8_t(b) & 0x7f) << by;
       by += 7;
   } while( uint8_t(b) & 0x80 );
   length_size = index;
   length = v;
}

extern "C" {
    void init()  {
       print("code::init");
    }

    void hello(uint64_t a, uint64_t b) {
       print(a,b);
       print("hello,word");
    }

    void test(int size)  {
       int a[size];
       read_message(a,sizeof(a));
    }

    /// The apply method implements the dispatch of events to this contract
    void apply( uint64_t code, uint64_t action ) {
       if( code == N(test) ) {
          if( action == N(test) ) {
             require_auth(N(test));
             const char *code = "def hello():\n" \
                          "    print('hello,world')";
             pythonLoad(N(hello),code,strlen(code));
             pythonCall(N(hello),N(hello),0,0);
          }
       }
    }
}

