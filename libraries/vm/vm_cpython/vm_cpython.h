#include <string>
#include <vector>
#include <eosiolib_native/vm_api.h>

using namespace std;

void get_code(uint64_t account, string& code);
int init_cpython_();
vm_api& api();
