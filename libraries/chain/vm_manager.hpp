#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <map>
#include <string>
#include <vector>
#include <memory>

using namespace std;

typedef int (*fn_setcode)(uint64_t account);
typedef int (*fn_apply)(uint64_t receiver, uint64_t account, uint64_t act);

struct vm_calls {
   void* handle;
   fn_setcode setcode;
   fn_apply apply;
};

class vm_manager
{
public:
   static vm_manager& get();
   int setcode(int type, uint64_t account);
   int apply(int type, uint64_t receiver, uint64_t account, uint64_t act);
private:
   vm_manager();
   map<int, std::unique_ptr<vm_calls>> vm_map;
};

