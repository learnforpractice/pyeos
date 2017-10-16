#include <boost/function.hpp>
#include <boost/multiprecision/cpp_bin_float.hpp>
#include <eos/chain/python_interface.hpp>
#include <eos/chain/wasm_interface.hpp>
#include <eos/chain/chain_controller.hpp>
#include "Platform/Platform.h"
#include "WAST/WAST.h"
#include "Runtime/Runtime.h"
#include "Runtime/Linker.h"
#include "Runtime/Intrinsics.h"
#include "IR/Module.h"
#include "IR/Operators.h"
#include "IR/Validate.h"
#include <eos/chain/key_value_object.hpp>
#include <eos/chain/account_object.hpp>
#include <chrono>

using namespace std;
int python_load(string& name, string& code);
int python_call(std::string &name, std::string &function,std::vector<uint64_t> args);


namespace eos {
namespace chain {
using namespace IR;
using namespace Runtime;
typedef boost::multiprecision::cpp_bin_float_50 DOUBLE;

int python_load_with_gil(string& name, string& code) {
   int ret = 0;
   PyGILState_STATE save = PyGILState_Ensure();
   try {
      ret = python_load(name, code);
   } catch (fc::assert_exception& e) {
      elog(e.to_detail_string());
   } catch (fc::exception& e) {
      elog(e.to_detail_string());
   } catch (boost::exception& ex) {
      elog(boost::diagnostic_information(ex));
   } catch (...) {
      elog("unhandled exception!!!");
   }
   PyGILState_Release(save);
   return ret;
}

int python_call_with_gil(std::string &name, std::string &function,
      std::vector<uint64_t> args) {
   int ret = 0;
   PyGILState_STATE save = PyGILState_Ensure();
   try {
      ret = python_call(name, function, args);
   } catch (fc::assert_exception& e) {
      elog(e.to_detail_string());
   } catch (fc::exception& e) {
      elog(e.to_detail_string());
   } catch (boost::exception& ex) {
      elog(boost::diagnostic_information(ex));
   } catch (...) {
      elog("unhandled exception!!!");
   }
   PyGILState_Release(save);
   return ret;
}

int wasm_call_function(uint64_t code, uint64_t function,
      std::vector<uint64_t> args) {
//   ilog("wasm_call_function args.size:${n}",("n",args.size()));
   for (auto& n : args) {
      ilog("${n}",("n",n));
   }
   apply_context* c = python_interface::get().current_apply_context;
   return wasm_interface::get().call_function(*c, code, function, args);

}

python_interface::python_interface() {
}

template<typename Function, typename KeyType, int numberOfKeys>
int32_t validate(int32_t valueptr, int32_t valuelen, Function func) {

   static const uint32_t keylen = numberOfKeys * sizeof(KeyType);

   FC_ASSERT(valuelen >= keylen, "insufficient data passed");

   auto& wasm = python_interface::get();
   FC_ASSERT(wasm.current_apply_context, "no apply context found");

   char* value = memoryArrayPtr<char>(wasm.current_memory, valueptr, valuelen);
   KeyType* keys = reinterpret_cast<KeyType*>(value);

   valuelen -= keylen;
   value += keylen;

   return func(wasm.current_apply_context, keys, value, valuelen);
}

python_interface& python_interface::get() {
   static python_interface* python = nullptr;
   if (!python) {
      wlog("python_interface::init");
      python = new python_interface();
   }
   return *python;
}

struct RootResolver: Runtime::Resolver {
   std::map<std::string, Resolver*> moduleNameToResolverMap;

   bool resolve(const std::string& moduleName, const std::string& exportName,
         ObjectType type, ObjectInstance*& outObject) override
         {
      // Try to resolve an intrinsic first.
      if (IntrinsicResolver::singleton.resolve(moduleName, exportName, type,
            outObject)) {
         return true;
      }
      FC_ASSERT(!"unresolvable", "${module}.${export}",
            ("module",moduleName)("export",exportName));
      return false;
   }
};

int64_t python_interface::current_execution_time() {
   return (fc::time_point::now() - checktimeStart).count();
}

char* python_interface::vm_allocate(int bytes) {
   return 0;
}

U32 python_interface::vm_pointer_to_offset(char* ptr) {
   return U32(ptr - &memoryRef<char>(current_memory, 0));
}

void python_interface::vm_call(const char* function_name) {
   std::vector<uint64_t> args = { current_validate_context->msg.code.value,
         current_validate_context->msg.type.value };

   string module_name = current_module;
   string function_name_ = function_name;

   python_call_with_gil(module_name, function_name_, args);
}

void python_interface::vm_apply() {
   vm_call("apply");
}

void python_interface::vm_onInit() {
   vector<uint64_t> args;
   string module_name = current_module;
   string function_name = "init";
   python_call_with_gil(module_name, function_name, args);
}

void python_interface::validate(apply_context& c) {
   /*
    current_validate_context       = &c;
    current_precondition_context   = nullptr;
    current_apply_context          = nullptr;

    load( c.code, c.db );
    vm_validate();
    */
}

void python_interface::precondition(apply_context& c) {
   try {

      /*
       current_validate_context       = &c;
       current_precondition_context   = &c;

       load( c.code, c.db );
       vm_precondition();
       */

   }FC_CAPTURE_AND_RETHROW()
}

void python_interface::apply(apply_context& c) {
   try {
      current_validate_context = &c;
      current_precondition_context = &c;
      current_apply_context = &c;

      load(c.code, c.db);
      vm_apply();

   }FC_CAPTURE_AND_RETHROW()
}

void python_interface::init(apply_context& c) {
   try {
      current_validate_context = &c;
      current_precondition_context = &c;
      current_apply_context = &c;
      load(c.code, c.db);
      vm_onInit();
   }FC_CAPTURE_AND_RETHROW()
}

void python_interface::load(const AccountName& name,
      const chainbase::database& db) {
   const auto& recipient = db.get<account_object, by_name>(name);
   string module_name = string(name);
   string code = string((const char*) recipient.code.data(),
         recipient.code.size());
   current_module = module_name;
   ilog("python_interface::load");
   python_load_with_gil(module_name, code);
}

}
}
