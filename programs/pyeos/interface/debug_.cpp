#include "debug_.hpp"
#include <libdevcore/CommonData.h>
#include <fc/log/logger.hpp>
#include <fc/variant_object.hpp>
#include <fc/reflect/reflect.hpp>

#include <fc/io/raw.hpp>

#include <fc/reflect/variant.hpp>
#include "json.hpp"
#include "pyobject.hpp"

#include <appbase/application.hpp>

using namespace dev;

using namespace fc;
using namespace std;

struct ABC {
   int a;
   int b;
//   int c;
   std::vector<char> c;
};

FC_REFLECT( ABC, (a)(b)(c) )

class visitor {
public:
   fc::mutable_variant_object& var;
   ABC& v;
   visitor(fc::mutable_variant_object& _var, ABC& _v) : var(_var), v(_v) {
   }
   template <typename Member, class Class, Member (Class::*member)>
   void operator() (const char* name) const {
      add(var, name, (v.*member)); // or an ugly but efficient solution: (*((fc::mutable_variant_object*)&var))(name, v.*member);
      wlog("${n}", ("n", v.*member));
   }
   template<typename M>
   void add(fc::mutable_variant_object& v, const char* name, M& m) const {
      v(name, m);
   }
};

//#include <fc/io/raw_fwd.hpp>
//#include <fc/io/raw_variant.hpp>

class pack_visitor {
public:
   ABC& v;
   fc::datastream<char*>& ds; //can not declare as fc::datastream<char*> ds, C++ is really tricky.
   pack_visitor(ABC& _v, fc::datastream<char*>& _ds) : v(_v), ds(_ds) {
   }
   template <typename Member, class Class, Member (Class::*member)>
   void operator() (const char*) const {
      pack(ds, v.*member); // can not call fc::raw::pack( ds, m); here since method is declared as "CONST"
//      ((fc::datastream<char*>*)&ds)->write((char*)&(v.*member), sizeof(Member)); //wrong implementation
      wlog("${n}", ("n", v.*member));
   }

   template<typename M>
   void pack(fc::datastream<char*>& _ds, M& m) const {
      fc::raw::pack( _ds, m);
   }
};


void debug_test() {
   std::vector<char> v;
   auto mvar = fc::mutable_variant_object()("a", v)("b", 123);
   mvar("d", 133.5);
   wlog("${a}, ${b}, ${d}", ("a", mvar["a"])("b", mvar["b"])("d", mvar["d"]));

   variant var;

   ABC a1 = {1, 2};
   ABC a2 = {3, 4};

   auto r = fc::reflector<ABC>();
   fc::reflector<ABC>::visit(visitor(mvar, a1));

   v.resize(sizeof(ABC));
   fc::datastream<char*> ds(v.data(), v.size());
   fc::reflector<ABC>::visit(pack_visitor(a1, ds));

   //defined in #include <fc/io/raw.hpp>
   fc::raw::pack_size(a1);
   fc::raw::pack(ds, a1);
   fc::raw::pack(a1);

   wlog("${n}", ("n", fc::variant(v).as_string()));

   fc::variant vv = fc::variant(a1); // or fc::to_variant(a1, vv); DONOT FORGET to include <fc/reflect/variant.hpp>
   auto vo = vv.get_object();
   wlog("${a}, ${b}", ("a", vo["a"])("b", vo["b"]));
   try {
      vo["d"];
   } catch (fc::key_not_found_exception& ex) {
      elog(ex.to_detail_string());
   }

   auto a3 = vv.as<ABC>();
   wlog("${a}, ${b}", ("a", a3.a)("b", a3.b));

//  the same
//   fc::to_variant(a1, vv);

   fc::to_variant(mvar, vv);
   fc::from_variant(vv, mvar);

   wlog("${n}", ("n", fc::variant(v).as_string()));

}

void debug_test2() {
   std::string s("abc.py");
   wlog("${n}", ("n", s.find(".py")));
   wlog("${n}", ("n", s.rfind(".py")));
}

void debug_test__() {
   u256 _key = u256(300);
   dev::bytes __key = dev::toBigEndian(_key);
   u256 ___key = dev::fromBigEndian<u256>(__key);
   wlog(___key.str());

}

static constexpr unsigned int DJBH(const char* cp)
{
  unsigned int hash = 5381;
  while (*cp)
      hash = 33 * hash ^ (unsigned char) *cp++;
  return hash;
}

uint64_t wasm_test_action_(const char* cls, const char* method)
{
  return static_cast<unsigned long long>(DJBH(cls)) << 32 | static_cast<unsigned long long>(DJBH(method));
}

#include <eosio/chain/block_log.hpp>
#include <eosio/chain/transaction.hpp>

using namespace eosio::chain;

int block_on_action(int block, PyObject* trx);

int block_on_raw_action(int block, string act);

#define FC_LOG_AND_RETURN( ... )  \
   catch( const boost::interprocess::bad_alloc& ) {\
      throw;\
   } catch( fc::exception& er ) { \
      wlog( "${details}", ("details",er.to_detail_string()) ); \
      return; \
   } catch( const std::exception& e ) {  \
      fc::exception fce( \
                FC_LOG_MESSAGE( warn, "rethrow ${what}: ",FC_FORMAT_ARG_PARAMS( __VA_ARGS__  )("what",e.what()) ), \
                fc::std_exception_code,\
                BOOST_CORE_TYPEID(e).name(), \
                e.what() ) ; \
      wlog( "${details}", ("details",fce.to_detail_string()) ); \
      return; \
   } catch( ... ) {  \
      fc::unhandled_exception e( \
                FC_LOG_MESSAGE( warn, "rethrow", FC_FORMAT_ARG_PARAMS( __VA_ARGS__) ), \
                std::current_exception() ); \
      wlog( "${details}", ("details",e.to_detail_string()) ); \
      return; \
   }


void block_log_test_(string& path, int start_block, int end_block) {
   eosio::chain::block_log log(path);
   for (int i=start_block;i<=end_block;i++) {
      signed_block_ptr block;
      try {
         block = log.read_block_by_num(i);
         if (!block) {
            wlog("bad block number ${n}", ("n", i));
            break;
         }
         for (auto& tr : block->transactions) {
            if (!tr.trx.contains<packed_transaction>()) {
               continue;
            }
            packed_transaction& pt = tr.trx.get<packed_transaction>();
            signed_transaction st = pt.get_signed_transaction();
            PyObject* json = python::json::to_string(fc::variant(st));
            int ret = block_on_action(i, json);
            if (!ret) {
               return;
            }
         }
      } FC_LOG_AND_RETURN();
   }
}

void block_log_get_actions_(string& path, int block_num) {
   eosio::chain::block_log log(path);

   signed_block_ptr block;
   try {
      block = log.read_block_by_num(block_num);
      if (!block) {
         wlog("bad block number ${n}", ("n", block_num));
         return;
      }
      for (auto& tr : block->transactions) {
         if (!tr.trx.contains<packed_transaction>()) {
            continue;
         }
         packed_transaction& pt = tr.trx.get<packed_transaction>();
         signed_transaction st = pt.get_signed_transaction();
         PyObject* json = python::json::to_string(fc::variant(st));
         int ret = block_on_action(block_num, json);
         if (!ret) {
            return;
         }
      }
   } FC_LOG_AND_RETURN();

}

void block_log_get_raw_actions_(string& path, int start, int end) {
   eosio::chain::block_log log(path);

   signed_block_ptr block;
   for (int block_num=start;block_num<end;block_num++) {
      try {
         block = log.read_block_by_num(block_num);
         if (!block) {
            wlog("bad block number ${n}", ("n", block_num));
            return;
         }
         for (auto& tr : block->transactions) {
            if (!tr.trx.contains<packed_transaction>()) {
               continue;
            }
            packed_transaction& pt = tr.trx.get<packed_transaction>();
            signed_transaction st = pt.get_signed_transaction();
            for (auto& act: st.actions) {
               auto v = fc::raw::pack(act);
               string raw_act(v.begin(), v.end());
               int ret = block_on_raw_action(block_num, raw_act);
               if (!ret) {
                  return;
               }
            }
         }
      } FC_LOG_AND_RETURN();
   }
}


PyObject* block_log_get_block_(string& path, int block_num) {
   eosio::chain::block_log log(path);
   signed_block_ptr block;
   try {
      block = log.read_block_by_num(block_num);
      if (!block) {
         wlog("bad block number ${n}", ("n", block_num));
         return py_new_none();
      }
      return python::json::to_string(fc::variant(block));
   } FC_LOG_AND_DROP();
   return py_new_none();
}

bool hash_option_(const char* option) {
   return appbase::app().has_option(option);
}
